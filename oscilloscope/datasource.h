// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QPointF>
#include <QVector>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries)
QT_FORWARD_DECLARE_CLASS(QQuickView)

/*!
 * \class DataSource
 * \brief C++ backend for the oscilloscope.
 *
 * A dedicated DRDY-interrupt thread (adcThreadFunc) reads the ADS1263 over SPI,
 * cycling through 4 single-ended channels (AIN0–AIN3 vs AINCOM).  Samples are
 * pushed into a mutex-protected ring buffer.  The QML 25 Hz refresh timer calls
 * updateChannel() from the main thread, which holds the same mutex while copying
 * data into a QtCharts series.
 *
 * On non-Linux hosts (or when /dev/spidev0.0 is unavailable) the thread falls
 * back to a synthetic signal generator so the UI still works on the desktop.
 *
 * Hardware: Waveshare High-Precision AD HAT (ADS1263)
 *   DRDY → BCM 17   CS → BCM 22   RST → BCM 18
 *   SPI  → /dev/spidev0.0, Mode 1, 4 MHz
 *   AINCOM wired to 2.5 V rail (= 12 V physical mid-rail)
 *   Voltage divider: 0–24 V physical → 0–5 V ADC input
 *   ADC1 bipolar, internal 2.5 V reference → ±2.5 V differential → ±12 V display
 */
class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(QQuickView *appViewer, QObject *parent = nullptr);
    ~DataSource();

    Q_INVOKABLE void setTriggerLevel(float level);
    Q_INVOKABLE void setTriggerChannel(int channel);
    Q_INVOKABLE void setSamplesPerView(int n);

    /*!
     * Sets the desired scrolling window duration in real seconds.
     * samplesPerView is recalibrated automatically from measured fps every
     * 200 frames so the window stays accurate regardless of actual throughput.
     */
    Q_INVOKABLE void setWindowSeconds(double s);

    /*!
     * Sets the EMA smoothing factor applied to each channel before the ring
     * buffer.  alpha=1.0 = no filter (raw); alpha=0.05 = heavy smoothing.
     * Useful for reducing high-frequency noise and inter-channel coupling
     * artifacts from the FIR filter on the ADS1263.
     */
    Q_INVOKABLE void setFilterAlpha(float alpha);
    Q_INVOKABLE void rearm();
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    /*!
     * Copies the latest \c m_samplesPerView samples for \a channel into
     * \a series.  Called by the QML 25 Hz timer on the main thread.
     * No-ops until the trigger has fired.
     */
    Q_INVOKABLE void updateChannel(int channel, QAbstractSeries *series);

    /*!
     * Returns the actual wall-clock duration (seconds) of the samples currently
     * in view.  Use this as axisX.max so the time axis reflects real elapsed
     * time rather than nominal ADC rate.
     */
    Q_INVOKABLE double viewDuration() const;

    /*!
     * Returns the measured frame rate (frames/s) estimated from timestamps in
     * the ring buffer.  Returns 0 if fewer than 100 frames have been collected.
     * Use this in setTimeRange to convert seconds → sample count correctly.
     */
    Q_INVOKABLE double measuredFrameRate() const;

    /*!
     * Exports the current view frame to CSV.
     * \a enabledChannels is a QVariantList of 4 bools (index 0–3); only
     * channels with true are written as columns.
     * \return true on success.
     */
    Q_INVOKABLE bool exportCsv(const QString &filepath, const QVariantList &enabledChannels);


signals:
    void frameReady();

private:
    static constexpr int kChannels = 4;

    // ── Ring buffer ───────────────────────────────────────────────────────
    struct ScopeRing {
        std::array<QVector<float>, kChannels> ch;
        QVector<double> timestamps; ///< steady_clock seconds relative to DataSource::m_startTime
        int size  = 0;
        int write = 0;

        void resize(int n) {
            size = n;
            for (auto &v : ch) v.resize(n);
            timestamps.resize(n, 0.0);
            write = 0;
        }

        void push(const std::array<float, kChannels> &s, double ts) {
            for (int c = 0; c < kChannels; ++c) ch[c][write] = s[c];
            timestamps[write] = ts;
            write = (write + 1) % size;
        }

        int lastIndex() const {
            if (size <= 0) return 0;
            int end = write - 1;
            return (end < 0) ? end + size : end;
        }
    };

    // ── Level trigger ─────────────────────────────────────────────────────
    // Fires as soon as the trigger channel reads >= level (no need to come
    // from below first).  Armed again only after an explicit rearm() call.
    struct Trigger {
        int   channel = 0;
        float level   = 1.0f;
        bool  armed   = true;

        bool update(float current) {
            if (!armed) return false;
            if (current >= level) { armed = false; return true; }
            return false;
        }
        void rearm() { armed = true; }
    };

    // ── Internal helpers ──────────────────────────────────────────────────
    void  armTrigger();
    bool  initHardware();
    void  shutdownHardware();
    void  adcThreadFunc();

    void  spiXfer(const uint8_t *tx, uint8_t *rx, int len);
    void  spiWriteCmd(uint8_t cmd);
    void  spiWriteReg(uint8_t reg, uint8_t data);

    // ── Members ───────────────────────────────────────────────────────────
    QQuickView *m_appViewer = nullptr;

    // Linux file descriptors (all -1 on non-Linux / init failure)
    int m_spiFd  = -1;   ///< /dev/spidev0.0
    int m_drdyFd = -1;   ///< BCM 17 sysfs value file (poll POLLPRI for falling edge)
    int m_csFd   = -1;   ///< BCM 22 sysfs value file (output, driven low for session)
    int m_rstFd  = -1;   ///< BCM 18 sysfs value file (output)

    std::thread       m_adcThread;
    std::atomic<bool> m_running{false};

    // Guards: m_ring, m_trigger, m_renderEnabled
    mutable std::mutex m_ringMutex;

    ScopeRing m_ring;
    Trigger   m_trigger;

    double m_windowSeconds       = 5.0;  ///< desired scroll window; drives auto-calibration
    int  m_samplesPerView       = 9000; ///< recalibrated automatically once fps is known

    // ── Per-channel EMA filter ────────────────────────────────────────────
    float m_emaAlpha = 1.0f;  ///< 1.0 = off; lower = more smoothing
    std::array<float, kChannels> m_emaState{}; ///< per-channel filter state
    bool m_renderEnabled        = false;
    int  m_samplesAfterTrigger  = 0;   ///< frames collected since trigger fired

    std::chrono::steady_clock::time_point m_startTime; ///< Set in start(); basis for ring timestamps

    float m_synthPhase = -4.0f; ///< Phase accumulator for synthetic fallback

    std::array<QVector<QPointF>, kChannels> m_scratchPoints;
};

#endif
