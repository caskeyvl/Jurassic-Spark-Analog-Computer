// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QList>
#include <QObject>
#include <QPointF>
#include <QTimer>

QT_FORWARD_DECLARE_CLASS(QAbstractSeries)
QT_FORWARD_DECLARE_CLASS(QQuickView)

/*!
 * \class DataSource
 * \brief The C++ backend for the oscilloscope. Manages sampling, buffering,
 *        trigger detection, and data delivery to the QML chart layer.
 *
 * DataSource runs a 1ms QTimer that continuously generates (or, in production,
 * reads) one sample per channel and pushes it into a circular ring buffer.
 * The QML ScopeView calls updateChannel() at ~25Hz to pull the latest frame
 * into a QtCharts series for rendering.
 *
 * Exposed to QML via QQmlContext::setContextProperty("dataSource", ...).
 */
class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(QQuickView *appViewer, QObject *parent = nullptr);

    /*! Sets the voltage level at which the trigger fires. */
    Q_INVOKABLE void setTriggerLevel(float level);

    /*! Sets which channel (0–3) the trigger monitors. */
    Q_INVOKABLE void setTriggerChannel(int channel);

    /*! Sets how many samples are displayed per frame. Minimum 16. */
    Q_INVOKABLE void setSamplesPerView(int n);

    /*! Re-arms the trigger so it waits for the next rising edge. */
    Q_INVOKABLE void rearm();

    /*! Starts the 1ms sampling timer. No-op if already running. */
    Q_INVOKABLE void start();

    /*! Stops the sampling timer, freezing the display. */
    Q_INVOKABLE void stop();

    /*!
     * Fills \a series with the latest \c samplesPerView samples from
     * \a channel. Called by the QML refresh timer for each enabled channel.
     * Does nothing if the trigger has not yet fired.
     */
    Q_INVOKABLE void updateChannel(int channel, QAbstractSeries *series);

    /*!
     * Exports the currently displayed frame to a CSV file at \a filepath.
     * Columns: time_s, ch1, ch2, ch3, ch4. Samples are written
     * oldest-to-newest, matching the order shown on screen.
     * \return true on success, false if the file could not be opened.
     */
    Q_INVOKABLE bool exportCsv(const QString &filepath);

signals:
    /*! Emitted after every sample, at the rate of the sampling timer. */
    void frameReady();

private slots:
    void sample();

private:
    static constexpr int kChannels = 4;

    /*!
     * \brief Circular ring buffer holding the most recent samples for all channels.
     *
     * Each channel occupies one QVector<float> of \c size elements. The write
     * head advances modulo \c size, so older samples are overwritten once the
     * buffer is full. Reading a contiguous view requires unwrapping the index
     * arithmetic — see fillSeries() and exportCsv().
     */
    struct ScopeRing {
        std::array<QVector<float>, kChannels> ch; /*!< Per-channel sample storage. */
        int size = 0;   /*!< Capacity of each channel buffer. */
        int write = 0;  /*!< Index where the next sample will be written. */

        /*! Allocates each channel buffer to \a n samples and resets the write head. */
        void resize(int n) {
            size = n;
            for (auto &v : ch) v.resize(n);
            write = 0;
        }

        /*! Writes one sample per channel from \a s and advances the write head. */
        void push(const std::array<float, kChannels>& s) {
            for(int c = 0; c < kChannels; ++c) ch[c][write] = s[c];
            write = (write + 1) % size;
        }

        /*!
         * Returns the index of the most recently written sample.
         * Since \c write points to the next write slot, the last written
         * slot is \c write-1 (wrapped).
         */
        int lastIndex() const {
            if (size <= 0) return 0;
            int end = write - 1;
            if (end < 0) end += size;
            return end;
        }
    };

    /*!
     * \brief Rising-edge trigger state machine.
     *
     * The trigger arms itself and waits for the monitored channel to cross
     * \c level from below. Once fired it disarms until rearm() is called,
     * preventing the display from re-triggering mid-frame.
     */
    struct Trigger {
        int channel = 0;    /*!< Index of the channel to monitor (0–3). */
        float level = 0.0f; /*!< Voltage threshold for the rising edge. */
        bool enabled = true;
        bool armed = true;
        float prev = 0.0f;  /*!< Sample value from the previous tick, used for edge detection. */

        /*!
         * Evaluates the trigger against \a current. Returns true (fires) when
         * \c prev was below \c level and \c current meets or exceeds it.
         * Disarms itself immediately after firing.
         */
        bool update(float current) {
            if(!enabled || !armed) { prev = current; return false; }
            bool fired = (prev < level && current >= level);
            prev = current;
            if(fired) armed = false;
            return fired;
        }

        /*! Re-arms the trigger so it can fire again on the next rising edge. */
        void rearm() { armed = true; }
    };

    /*! Re-arms the trigger and suppresses rendering until the next trigger event. */
    void armTrigger();

    QQuickView *m_appViewer = nullptr;
    QTimer m_sampleTimer;   /*!< Fires every 1ms to drive the sampling loop. */

    ScopeRing m_ring;       /*!< Circular buffer holding all channel data. */
    Trigger m_trigger;

    int m_samplesPerView = 5000; /*!< Number of samples shown in one screen frame. */
    bool m_renderEnabled = false; /*!< Set true by the trigger; gates updateChannel(). */

    /*! Per-channel scratch buffers for QPointF data passed to QtCharts. */
    std::array<QVector<QPointF>, kChannels> m_scratchPoints;
};

#endif
