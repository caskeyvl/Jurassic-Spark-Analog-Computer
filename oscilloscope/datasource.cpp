// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "datasource.h"

#include <QAreaSeries>
#include <QQuickItem>
#include <QQuickView>
#include <QXYSeries>
#include <QtMath>       // qSin/qCos for synthetic signal generation; remove when ADC reads replace sample()
#include <QFile>
#include <QTextStream>

Q_DECLARE_METATYPE(QAbstractSeries *)
Q_DECLARE_METATYPE(QAbstractAxis *)

/*!
 * \brief Unwraps \a N samples from a circular buffer into a QXYSeries.
 *
 * Reads from \a buf (size \a ringSize) ending at \a endIndex, converting
 * each sample to a QPointF with X = sample index * 0.001 (seconds) and
 * Y = sample value. Results are staged in \a scratch to avoid repeated
 * heap allocation, then pushed to the series in one replace() call.
 *
 * \note This is the canonical ring-unwrap used by both rendering and export.
 *       Keep exportCsv() index arithmetic in sync with this function.
 */
static void fillSeries(QXYSeries* xy,
                       const QVector<float>& buf,
                       int ringSize,
                       int endIndex,
                       int N,
                       QVector<QPointF>& scratch)
{
    if (!xy || ringSize <= 0 || N <= 1) return;

    scratch.resize(N);

    int start = endIndex - (N - 1);
    start %= ringSize; if (start < 0) start += ringSize;

    for(int i = 0; i < N; ++i) {
        int idx = start + i;
        if(idx >= ringSize) idx -= ringSize;
        scratch[i].setX(i * 0.001f);
        scratch[i].setY(buf[idx]);
    }

    xy->replace(scratch);
}

DataSource::DataSource(QQuickView *appViewer, QObject *parent)
    : QObject(parent)
    , m_appViewer(appViewer)
{
    m_ring.resize(8192);    // todo: resize to 10000 if reasonable (after ADC integration)

    connect(&m_sampleTimer, &QTimer::timeout, this, &DataSource::sample);
    m_sampleTimer.start(1);
}

void DataSource::armTrigger() {
    m_trigger.rearm();
    m_renderEnabled = false;
}

void DataSource::rearm() {
    armTrigger();
}

void DataSource::setTriggerLevel(float level) {
    m_trigger.level = level;
}


void DataSource::setSamplesPerView(int n) {
    if (n < 16) n = 16;
    m_samplesPerView = n;
}

void DataSource::start(){ if (!m_sampleTimer.isActive()) m_sampleTimer.start(1); }

void DataSource::stop(){ m_sampleTimer.stop(); }

void DataSource::updateChannel(int channel, QAbstractSeries* series) {
    if(!m_renderEnabled) return;
    if(!series) return;
    if(channel < 0 || channel >= kChannels) return;

    auto *xy = qobject_cast<QXYSeries *>(series);
    if(!xy) return;

    const int ringSize = m_ring.size;
    if(ringSize <= 1) return;

    const int N = qMin(m_samplesPerView, ringSize);
    if (N <= 1) return;

    const int end = m_ring.lastIndex();

    fillSeries(xy, m_ring.ch[channel], ringSize, end, N, m_scratchPoints[channel]);
}


void DataSource::sample() {
    // TODO: replace synthetic waveforms with real ADC reads.
    // Ch1: linear ramp  Ch2: scaled ramp  Ch3: sine  Ch4: cosine (different frequency)
    static float phase = -4.0f;
    phase += 0.02f;
    if (phase > 8.0f) phase = -1.0f;

    const float ch1 = phase;
    const float ch2 = phase * 0.5f + 1.0f;
    const float ch3 = qSin(phase) * 2.0f;
    const float ch4 = qCos(phase * 0.7f) * 1.5f;

    const std::array<float, kChannels> s { ch1, ch2, ch3, ch4};
    m_ring.push(s);

    // Gate rendering on the trigger: once the trigger fires, updateChannel()
    // is allowed to draw until rearm() is called again.
    const float trigSample = s[m_trigger.channel];
    if (!m_renderEnabled && m_trigger.update(trigSample))
        m_renderEnabled = true;

    emit frameReady();
}

void DataSource::setTriggerChannel(int channel) {
    if(channel < 0) channel = 0;
    if(channel >= kChannels) channel = kChannels - 1;

    m_trigger.channel = channel;
}

/*!
 * Exports the currently displayed frame to a CSV file at \a filepath.
 * Columns: time_s, ch1, ch2, ch3, ch4. Samples are written oldest-to-newest,
 * matching the order shown on screen.
 * \return true on success, false if the file could not be opened.
 */
bool DataSource::exportCsv(const QString &filepath) {
    const int ringSize = m_ring.size; 
    const int N = qMin(m_samplesPerView, ringSize);
    if (N <= 1) return false; 
    
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    
    QTextStream out(&file);
    out << "time_s,ch1,ch2,ch3,ch4\n"; 

    // Unwrap ring buffer starting from oldest sample in current view
    // Same index calculations as done in fillSeries()
    const int end = m_ring.lastIndex();
    int start = end - (N - 1); 
    start %= ringSize;
    if (start < 0) start += ringSize; 
    
    for (int i = 0; i < N; ++i) { 
        int idx = start + i; 
        if (idx >= ringSize) idx -= ringSize;

        out << (i * 0.001f);        // ~1 ms per sample 
        for (int c = 0; c < kChannels; ++c) { 
            out << ',' << m_ring.ch[c][idx];
        }
        out << '\n'; 
    }
    return true; 
}

