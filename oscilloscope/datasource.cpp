// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "datasource.h"

#ifdef __linux__
#  include <cerrno>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <fcntl.h>
#  include <linux/spi/spidev.h>
#  include <poll.h>
#  include <sys/ioctl.h>
#  include <sys/stat.h>
#  include <unistd.h>
#endif

#include <QAreaSeries>
#include <QDebug>
#include <QFile>
#include <QQuickItem>
#include <QQuickView>
#include <QTextStream>
#include <QXYSeries>
#include <QtMath>
//#include <QBluetoothLocalDevice>

Q_DECLARE_METATYPE(QAbstractSeries *)
Q_DECLARE_METATYPE(QAbstractAxis *)

// ── ADS1263 constants ─────────────────────────────────────────────────────
//
// Hardware setup:
//   AINCOM = 2.5 V (physical 12 V mid-rail, COM jumper removed, 2.5 V ref wired to COM terminal)
//   Voltage divider: 0–24 V → 0–5 V at ADC input
//   ADC differential: (AINx − AINCOM) ∈ [−2.5 V, +2.5 V]
//   Internal 2.5 V reference → code = Vdiff / 2.5 × 2^31  (signed 32-bit)
//
//   Voltage displayed (relative to 12 V mid-rail):
//     V = int32_t(raw) × (12.0 / 2^31)
//
static constexpr float kPhysicalScale = 12.0f; // V_display = signed_raw × kPhysicalScale / 2^31

// Register addresses
static constexpr uint8_t kRegMode0  = 0x03;
static constexpr uint8_t kRegMode1  = 0x04;
static constexpr uint8_t kRegMode2  = 0x05;
static constexpr uint8_t kRegInpMux = 0x06;
static constexpr uint8_t kRegRefMux = 0x0F;

// Commands
static constexpr uint8_t kCmdReset  = 0x06;
static constexpr uint8_t kCmdStop1  = 0x0A;
static constexpr uint8_t kCmdStart1 = 0x08;
static constexpr uint8_t kCmdRData1 = 0x12;
static constexpr uint8_t kCmdRReg   = 0x20;
static constexpr uint8_t kCmdWReg   = 0x40;

// INPMUX per channel: AIN0–AIN3 vs AINCOM (0x0A)
// Encoding: bits[7:4] = MUXP (channel index), bits[3:0] = MUXN (0xA = AINCOM)
static constexpr uint8_t kMuxTable[4] = { 0x0A, 0x1A, 0x2A, 0x3A };

// GPIO BCM pin numbers (Waveshare High-Precision AD HAT)
static constexpr int kGpioDrdy = 17;
static constexpr int kGpioCs   = 22;
static constexpr int kGpioRst  = 18;

// Effective sample rate delivered to the ring buffer.
// ADS1263 ADC1 is configured for 7 200 SPS (MODE2 = 0x8C).
// Cycling 4 channels → one complete 4-channel frame every 4 conversions.
static constexpr float kFrameRate   = 7200.0f / 4; // 1 800 frames/s (4 channels)
static constexpr float kFramePeriod = 1.0f / kFrameRate;               // seconds per frame

// ── fillSeries ────────────────────────────────────────────────────────────
/*!
 * Unwraps \a N samples from circular buffer \a buf (size \a ringSize, newest
 * at \a endIndex) into \a scratch and pushes them to \a xy in one replace()
 * call.  X axis is actual wall-clock seconds (timestamps[first] == 0);
 * Y axis is in display volts.
 */
static void fillSeries(QXYSeries *xy,
                       const QVector<float> &buf,
                       const QVector<double> &timestamps,
                       int ringSize, int endIndex, int N,
                       QVector<QPointF> &scratch)
{
    if (!xy || ringSize <= 0 || N <= 1) return;

    scratch.resize(N);
    int start = endIndex - (N - 1);
    start %= ringSize;
    if (start < 0) start += ringSize;

    const double t0 = timestamps[start]; // X = 0 at the first visible sample
    for (int i = 0; i < N; ++i) {
        int idx = start + i;
        if (idx >= ringSize) idx -= ringSize;
        scratch[i].setX(timestamps[idx] - t0);
        scratch[i].setY(buf[idx]);
    }
    xy->replace(scratch);
}

// ── Linux GPIO sysfs helpers ──────────────────────────────────────────────
#ifdef __linux__

#include <dirent.h>

// Newer RPi kernels (6.6+) assign a non-zero base to the GPIO chip, so the
// sysfs number for BCM pin N is (base + N), not just N.  Read it once at
// start-up by scanning /sys/class/gpio/gpiochip*/base.
static int gpioChipBase()
{
    int base = 0;
    DIR *dir = ::opendir("/sys/class/gpio");
    if (!dir) return base;

    struct dirent *ent;
    while ((ent = ::readdir(dir)) != nullptr) {
        if (::strncmp(ent->d_name, "gpiochip", 8) != 0) continue;
        char path[128];
        ::snprintf(path, sizeof(path), "/sys/class/gpio/%s/base", ent->d_name);
        int fd = ::open(path, O_RDONLY);
        if (fd < 0) continue;
        char buf[16] = {};
        ::read(fd, buf, sizeof(buf) - 1);
        ::close(fd);
        int candidate = std::atoi(buf);
        if (candidate > base) base = candidate; // take the highest (main chip)
    }
    ::closedir(dir);
    return base;
}

static void gpioExport(int gpio)
{
    // If already exported (e.g. previous run crashed), unexport first so we
    // get a clean state rather than inheriting stale direction/edge settings.
    char dirPath[64];
    ::snprintf(dirPath, sizeof(dirPath), "/sys/class/gpio/gpio%d", gpio);
    struct stat st{};
    if (::stat(dirPath, &st) == 0) {
        int ufd = ::open("/sys/class/gpio/unexport", O_WRONLY);
        if (ufd >= 0) {
            char buf[8];
            int n = ::snprintf(buf, sizeof(buf), "%d", gpio);
            ::write(ufd, buf, n);
            ::close(ufd);
            ::usleep(100'000);
        }
    }

    int fd = ::open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) return;
    char buf[8];
    int n = ::snprintf(buf, sizeof(buf), "%d", gpio);
    ::write(fd, buf, n);
    ::close(fd);
    ::usleep(150'000); // 150 ms — let the kernel create the sysfs directory
}

static void gpioSetAttr(int gpio, const char *attr, const char *value)
{
    char path[64];
    ::snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/%s", gpio, attr);
    int fd = ::open(path, O_WRONLY);
    if (fd < 0) return;
    ::write(fd, value, ::strlen(value));
    ::close(fd);
}

static int gpioOpenValue(int gpio, int flags)
{
    char path[64];
    ::snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    return ::open(path, flags);
}

static void gpioWrite(int fd, bool high)
{
    if (fd < 0) return;
    ::write(fd, high ? "1" : "0", 1);
}

#endif // __linux__

// ── DataSource ────────────────────────────────────────────────────────────

DataSource::DataSource(QQuickView *appViewer, QObject *parent)
    : QObject(parent)
    , m_appViewer(appViewer)
{
    // Ring buffer: ~55 s at 1 800 frames/s — plenty for any view window
    m_ring.resize(100'000);
}

DataSource::~DataSource()
{
    stop();
}

// ── Hardware init / shutdown ──────────────────────────────────────────────

bool DataSource::initHardware()
{
#ifdef __linux__
    // ── SPI ──────────────────────────────────────────────────────────────
    m_spiFd = ::open("/dev/spidev0.0", O_RDWR);
    if (m_spiFd < 0) {
        qWarning("ADS1263: cannot open /dev/spidev0.0 (%s) — synthetic data active",
                 strerror(errno));
        return false;
    }
    // Mode 1 (CPOL=0, CPHA=1), 4 MHz, no kernel-managed CS (CS is on GPIO 22)
    uint8_t  mode  = SPI_MODE_1 | SPI_NO_CS;
    uint32_t speed = 4'000'000;
    uint8_t  bits  = 8;
    ::ioctl(m_spiFd, SPI_IOC_WR_MODE,          &mode);
    ::ioctl(m_spiFd, SPI_IOC_WR_MAX_SPEED_HZ,  &speed);
    ::ioctl(m_spiFd, SPI_IOC_WR_BITS_PER_WORD, &bits);

    // ── GPIO ─────────────────────────────────────────────────────────────
    // Newer RPi kernels offset sysfs GPIO numbers by the chip base (e.g. 512).
    const int base = gpioChipBase();
    const int sysDrdy = base + kGpioDrdy;
    const int sysCs   = base + kGpioCs;
    const int sysRst  = base + kGpioRst;
    qDebug("ADS1263: GPIO chip base = %d (DRDY=%d CS=%d RST=%d)",
           base, sysDrdy, sysCs, sysRst);

    gpioExport(sysDrdy);
    gpioExport(sysCs);
    gpioExport(sysRst);

    gpioSetAttr(sysDrdy, "direction", "in");
    gpioSetAttr(sysDrdy, "edge",      "falling");   // interrupt on DRDY ↓

    gpioSetAttr(sysCs,  "direction", "out");
    gpioSetAttr(sysRst, "direction", "out");

    m_drdyFd = gpioOpenValue(sysDrdy, O_RDONLY | O_NONBLOCK);
    m_csFd   = gpioOpenValue(sysCs,   O_WRONLY);
    m_rstFd  = gpioOpenValue(sysRst,  O_WRONLY);

    if (m_drdyFd < 0 || m_csFd < 0 || m_rstFd < 0) {
        qWarning("ADS1263: GPIO sysfs init failed (base=%d) — synthetic data active", base);
        return false;
    }

    // ── Hardware reset ────────────────────────────────────────────────────
    gpioWrite(m_rstFd, true);  ::usleep(200'000);
    gpioWrite(m_rstFd, false); ::usleep(200'000);
    gpioWrite(m_rstFd, true);  ::usleep(200'000);

    // Assert CS and hold it low for the entire ADC session.
    // The ADS1263 SPI protocol allows CS to remain asserted across commands.
    gpioWrite(m_csFd, false);

    // ── Software reset & stop ─────────────────────────────────────────────
    spiWriteCmd(kCmdReset);
    ::usleep(4'000); // 4 ms — mandatory reset recovery time (TRES)
    spiWriteCmd(kCmdStop1);

    // ── Verify chip ID (REG_ID[7:5] should be 0x01) ───────────────────────
    uint8_t idTx[3] = { static_cast<uint8_t>(kCmdRReg | 0x00), 0x00, 0x00 };
    uint8_t idRx[3] = {};
    spiXfer(idTx, idRx, 3);
    uint8_t chipId = idRx[2] >> 5;
    if (chipId != 0x01)
        qWarning("ADS1263: unexpected chip ID 0x%02X (expected 0x01) — continuing", chipId);
    else
        qDebug("ADS1263: chip ID OK");

    // ── Configure registers ───────────────────────────────────────────────
    //
    // MODE2 (0x05): PGA bypass (bit7=1), gain=1 (bits6:4=000), 7200 SPS (bits3:0=0xC)
    //   0x80 | 0x0C = 0x8C
    //   At 7200 SPS and 4 channels: ~1800 frames/s per channel.
    //   Comfortable timing margin for sysfs GPIO overhead on Pi Zero 2W.
    spiWriteReg(kRegMode2, 0x8C);

    // MODE0: bit6=1 → pulse mode (ADC waits for START1 each conversion, no auto-restart).
    //   Pulse mode eliminates the MUX race: we write INPMUX, then fire START1, so the
    //   ADC always converts the channel we just selected.  bits[1:0]=00 → CHK disabled.
    spiWriteReg(kRegMode0, 0x40);

    // MODE1 (0x04): FIR digital filter — best noise rejection for DC/low-frequency.
    //   0x84 per Waveshare driver.
    spiWriteReg(kRegMode1, 0x84);

    // REFMUX (0x0F): internal 2.5 V reference (reset default 0x00 — write explicitly).
    spiWriteReg(kRegRefMux, 0x00);

    // INPMUX (0x06): start on AIN0 vs AINCOM.
    spiWriteReg(kRegInpMux, kMuxTable[0]);

    // Kick off the first conversion (pulse mode — subsequent START1s sent by thread).
    spiWriteCmd(kCmdStart1);
    // CS stays low — released only in shutdownHardware()

    return true;
#else
    return false;
#endif
}

void DataSource::shutdownHardware()
{
#ifdef __linux__
    if (m_csFd  >= 0) { gpioWrite(m_csFd, true);  ::close(m_csFd);  m_csFd  = -1; }
    if (m_rstFd >= 0) {                             ::close(m_rstFd); m_rstFd = -1; }
    if (m_drdyFd >= 0) {                            ::close(m_drdyFd); m_drdyFd = -1; }
    if (m_spiFd  >= 0) {                            ::close(m_spiFd);  m_spiFd  = -1; }
#endif
}

// ── SPI helpers ───────────────────────────────────────────────────────────

void DataSource::spiXfer(const uint8_t *tx, uint8_t *rx, int len)
{
#ifdef __linux__
    struct spi_ioc_transfer tr = {};
    tr.tx_buf        = reinterpret_cast<unsigned long>(tx);
    tr.rx_buf        = reinterpret_cast<unsigned long>(rx);
    tr.len           = static_cast<uint32_t>(len);
    tr.speed_hz      = 4'000'000;
    tr.bits_per_word = 8;
    ::ioctl(m_spiFd, SPI_IOC_MESSAGE(1), &tr);
#else
    Q_UNUSED(tx); Q_UNUSED(rx); Q_UNUSED(len);
#endif
}

void DataSource::spiWriteCmd(uint8_t cmd)
{
    spiXfer(&cmd, nullptr, 1);
}

void DataSource::spiWriteReg(uint8_t reg, uint8_t data)
{
    uint8_t tx[3] = { static_cast<uint8_t>(kCmdWReg | reg), 0x00, data };
    spiXfer(tx, nullptr, 3);
}

// ── ADC thread ────────────────────────────────────────────────────────────

void DataSource::adcThreadFunc()
{
    const bool useReal = (m_spiFd >= 0 && m_drdyFd >= 0 && m_csFd >= 0);

    if (useReal) {
#ifdef __linux__
        int currentCh = 0;
        std::array<float, kChannels> pending{};
        int chDone = 0;

        // Prime sysfs interrupt: consume any pending edge before entering the loop
        char edgeBuf[4];
        ::lseek(m_drdyFd, 0, SEEK_SET);
        ::read(m_drdyFd, edgeBuf, sizeof(edgeBuf));

        while (m_running) {
            // ── Block until DRDY falls ────────────────────────────────────
            // poll() waits for POLLPRI, which sysfs asserts on each GPIO edge.
            struct pollfd pfd = { m_drdyFd, POLLPRI | POLLERR, 0 };
            int ret = ::poll(&pfd, 1, 500); // 500 ms timeout so stop() can exit cleanly

            if (!m_running) break;

            // Consume the edge (mandatory before next poll() will arm correctly)
            ::lseek(m_drdyFd, 0, SEEK_SET);
            ::read(m_drdyFd, edgeBuf, sizeof(edgeBuf));

            if (ret <= 0 || !(pfd.revents & POLLPRI))
                continue; // timeout or spurious wakeup — go wait again

            // ── Read ADC1: RDATA1 command + 6-byte response ───────────────
            // TX: [0x12, 0x00 × 6]
            // RX: [don't-care, STATUS, DATA[31:24], DATA[23:16], DATA[15:8], DATA[7:0], CRC]
            uint8_t tx[7] = { kCmdRData1, 0, 0, 0, 0, 0, 0 };
            uint8_t rx[7] = {};
            spiXfer(tx, rx, 7);

            // Reconstruct 32-bit signed result (big-endian, bytes 2–5)
            uint32_t rawU = (uint32_t(rx[2]) << 24)
                          | (uint32_t(rx[3]) << 16)
                          | (uint32_t(rx[4]) <<  8)
                          |  uint32_t(rx[5]);

            // V_display = signed_raw × 12 / 2^31  (volts relative to 12 V mid-rail)
            pending[currentCh] = static_cast<float>(static_cast<int32_t>(rawU))
                                * (kPhysicalScale / 2147483648.0f);

            // ── Advance MUX to next channel and start next conversion ────────
            // Pulse mode: write INPMUX first, then START1.  The ADC won't begin
            // converting until START1 arrives, so there is no race on INPMUX.
            currentCh = (currentCh + 1) % kChannels;
            spiWriteReg(kRegInpMux, kMuxTable[currentCh]);
            spiWriteCmd(kCmdStart1);

            // ── Push frame once all 4 channels collected ──────────────────
            if (++chDone == kChannels) {
                chDone = 0;
                const double ts = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - m_startTime).count();
                {
                    std::lock_guard<std::mutex> lock(m_ringMutex);
                    // Per-channel EMA: smooths noise without mixing channels.
                    if (m_emaAlpha < 1.0f) {
                        for (int c = 0; c < kChannels; ++c) {
                            m_emaState[c] = m_emaAlpha * pending[c]
                                          + (1.0f - m_emaAlpha) * m_emaState[c];
                            pending[c] = m_emaState[c];
                        }
                    }
                    m_ring.push(pending, ts);
                    if (!m_renderEnabled
                            && m_trigger.update(pending[m_trigger.channel]))
                        m_renderEnabled = true;
                    if (m_renderEnabled) {
                        ++m_samplesAfterTrigger;
                        // Recalibrate every 200 frames so the window stays
                        // accurate regardless of actual SPI/GPIO throughput.
                        if (m_windowSeconds > 0 && m_samplesAfterTrigger % 200 == 0) {
                            const int CN = qMin(200, m_ring.size);
                            const int cEnd = m_ring.lastIndex();
                            int cStart = cEnd - (CN - 1);
                            if (cStart < 0) cStart += m_ring.size;
                            const double dt = m_ring.timestamps[cEnd] - m_ring.timestamps[cStart];
                            if (dt > 0.0)
                                m_samplesPerView = qMax(50, static_cast<int>(m_windowSeconds * (CN - 1) / dt));
                        }
                    }
                }
                emit frameReady(); // queued connection: safe from non-Qt thread
            }
        }
#endif // __linux__

    } else {
        // ── Synthetic fallback (desktop builds / no SPI device) ───────────
        while (m_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            m_synthPhase += 0.02f;
            if (m_synthPhase > 8.0f) m_synthPhase = -1.0f;

            std::array<float, kChannels> s = {
                m_synthPhase,
                m_synthPhase * 0.5f + 1.0f,
                qSin(m_synthPhase) * 2.0f,
                qCos(m_synthPhase * 0.7f) * 1.5f
            };
            const double ts = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - m_startTime).count();
            {
                std::lock_guard<std::mutex> lock(m_ringMutex);
                if (m_emaAlpha < 1.0f) {
                    for (int c = 0; c < kChannels; ++c) {
                        m_emaState[c] = m_emaAlpha * s[c]
                                      + (1.0f - m_emaAlpha) * m_emaState[c];
                        s[c] = m_emaState[c];
                    }
                }
                m_ring.push(s, ts);
                if (!m_renderEnabled && m_trigger.update(s[m_trigger.channel]))
                    m_renderEnabled = true;
                if (m_renderEnabled) {
                    ++m_samplesAfterTrigger;
                    if (m_windowSeconds > 0 && m_samplesAfterTrigger % 200 == 0) {
                        const int CN = qMin(200, m_ring.size);
                        const int cEnd = m_ring.lastIndex();
                        int cStart = cEnd - (CN - 1);
                        if (cStart < 0) cStart += m_ring.size;
                        const double dt = m_ring.timestamps[cEnd] - m_ring.timestamps[cStart];
                        if (dt > 0.0)
                            m_samplesPerView = qMax(50, static_cast<int>(m_windowSeconds * (CN - 1) / dt));
                    }
                }
            }
            emit frameReady();
        }
    }
}

// ── Public interface ──────────────────────────────────────────────────────

void DataSource::start()
{
    if (m_running) return;
    armTrigger(); // reset trigger, renderEnabled, and sample counter before each new capture
    m_startTime = std::chrono::steady_clock::now();
    m_running = true;
    initHardware(); // sets m_spiFd / GPIO fds; failure leaves them at -1 → synthetic
    m_adcThread = std::thread(&DataSource::adcThreadFunc, this);
}

void DataSource::stop()
{
    if (!m_running) return;
    m_running = false;
    if (m_adcThread.joinable())
        m_adcThread.join(); // at most 500 ms (poll timeout)
    shutdownHardware();
}

void DataSource::armTrigger()
{
    std::lock_guard<std::mutex> lock(m_ringMutex);
    m_trigger.rearm();
    m_renderEnabled = false;
    m_samplesAfterTrigger = 0;
    m_emaState.fill(0.0f);
}

void DataSource::rearm() { armTrigger(); }

void DataSource::setTriggerLevel(float level)
{
    std::lock_guard<std::mutex> lock(m_ringMutex);
    m_trigger.level = level;
}

void DataSource::setTriggerChannel(int channel)
{
    if (channel < 0)          channel = 0;
    if (channel >= kChannels) channel = kChannels - 1;
    std::lock_guard<std::mutex> lock(m_ringMutex);
    m_trigger.channel = channel;
}

void DataSource::setSamplesPerView(int n)
{
    if (n < 16) n = 16;
    std::lock_guard<std::mutex> lock(m_ringMutex);
    m_windowSeconds = 0; // manual override — disable auto-calibration
    m_samplesPerView = n;
}

void DataSource::setFilterAlpha(float alpha)
{
    // Clamp to (0, 1]; 0 would freeze the filter output permanently.
    if (alpha <= 0.0f) alpha = 0.01f;
    if (alpha >  1.0f) alpha = 1.0f;
    std::lock_guard<std::mutex> lock(m_ringMutex);
    m_emaAlpha = alpha;
    m_emaState.fill(0.0f); // reset state so old data doesn't drag on new alpha
}

void DataSource::setWindowSeconds(double s)
{
    std::lock_guard<std::mutex> lock(m_ringMutex);
    m_windowSeconds = s;
    // Immediate best-effort calibration if we already have data.
    if (s > 0 && m_samplesAfterTrigger > 50) {
        const int N = qMin(m_samplesAfterTrigger, qMin(1000, m_ring.size));
        const int endIdx = m_ring.lastIndex();
        int startIdx = endIdx - (N - 1);
        startIdx %= m_ring.size;
        if (startIdx < 0) startIdx += m_ring.size;
        const double dt = m_ring.timestamps[endIdx] - m_ring.timestamps[startIdx];
        if (dt > 0.0)
            m_samplesPerView = qMax(50, static_cast<int>(s * (N - 1) / dt));
    }
}

void DataSource::updateChannel(int channel, QAbstractSeries *series)
{
    if (channel < 0 || channel >= kChannels || !series) return;
    auto *xy = qobject_cast<QXYSeries *>(series);
    if (!xy) return;

    std::lock_guard<std::mutex> lock(m_ringMutex);
    if (!m_renderEnabled) return;

    const int ringSize = m_ring.size;
    if (ringSize <= 1) return;

    const int N = qMin(qMin(m_samplesAfterTrigger, m_samplesPerView), ringSize);
    if (N <= 1) return;

    fillSeries(xy, m_ring.ch[channel], m_ring.timestamps,
               ringSize, m_ring.lastIndex(), N, m_scratchPoints[channel]);
}

double DataSource::viewDuration() const
{
    std::lock_guard<std::mutex> lock(m_ringMutex);
    if (!m_renderEnabled || m_samplesAfterTrigger < 2) return 0.0;
    const int N = qMin(qMin(m_samplesAfterTrigger, m_samplesPerView), m_ring.size);
    if (N < 2) return 0.0;
    const int endIdx = m_ring.lastIndex();
    int startIdx = endIdx - (N - 1);
    startIdx %= m_ring.size;
    if (startIdx < 0) startIdx += m_ring.size;
    return m_ring.timestamps[endIdx] - m_ring.timestamps[startIdx];
}

double DataSource::measuredFrameRate() const
{
    std::lock_guard<std::mutex> lock(m_ringMutex);
    if (m_samplesAfterTrigger < 100) return 0.0;
    // Use up to the last 1000 frames in the ring for a stable estimate.
    const int N = qMin(1000, qMin(m_samplesAfterTrigger, m_ring.size));
    const int endIdx = m_ring.lastIndex();
    int startIdx = endIdx - (N - 1);
    startIdx %= m_ring.size;
    if (startIdx < 0) startIdx += m_ring.size;
    const double dt = m_ring.timestamps[endIdx] - m_ring.timestamps[startIdx];
    if (dt <= 0.0) return 0.0;
    return (N - 1) / dt;
}

bool DataSource::exportCsv(const QString &filepath, const QVariantList &enabledChannels)
{
    // Build a list of which channel indices to export.
    QVector<int> cols;
    for (int c = 0; c < kChannels; ++c) {
        if (c < enabledChannels.size() && enabledChannels[c].toBool())
            cols.append(c);
    }
    if (cols.isEmpty()) return false;

    std::lock_guard<std::mutex> lock(m_ringMutex);
    const int ringSize = m_ring.size;
    const int N = qMin(m_samplesPerView, ringSize);
    if (N <= 1) return false;

    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "time_s";
    for (int c : cols) out << ",ch" << (c + 1);
    out << '\n';

    const int end = m_ring.lastIndex();
    int start = end - (N - 1);
    start %= ringSize;
    if (start < 0) start += ringSize;

    const double t0 = m_ring.timestamps[start];
    for (int i = 0; i < N; ++i) {
        int idx = start + i;
        if (idx >= ringSize) idx -= ringSize;
        out << (m_ring.timestamps[idx] - t0);
        for (int c : cols) out << ',' << m_ring.ch[c][idx];
        out << '\n';
    }
    return true;
}

