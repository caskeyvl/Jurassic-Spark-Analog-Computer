// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "datasource.h"

#include <QAreaSeries>
#include <QQuickItem>
#include <QQuickView>
#include <QRandomGenerator>
#include <QtMath>
#include <QXYSeries>

Q_DECLARE_METATYPE(QAbstractSeries *)
Q_DECLARE_METATYPE(QAbstractAxis *)

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
        scratch[i].setX(i);
        scratch[i].setY(buf[idx]);
    }

    xy->replace(scratch);
}
DataSource::DataSource(QQuickView *appViewer, QObject *parent)
    : QObject(parent)
    , m_appViewer(appViewer)
{
    m_ring.resize(8192);

    connect(&m_sampleTimer, &QTimer::timeout, this, &DataSource::sample);
    m_sampleTimer.start(1);
}

void DataSource::armTrigger() {
    m_trigger.rearm();
    m_renderEnabled = false;
}

void DataSource::setTriggerLevel(float level) {
    m_trigger.level = level;
    armTrigger();
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
    // Replace with ADC reads after
    static float phase = -4.0f;
    phase += 0.02f;
    if (phase > 8.0f) phase = -1.0f;

    const float ch1 = phase;
    const float ch2 = phase * 0.5f + 1.0f;
    const float ch3 = qSin(phase) * 2.0f;
    const float ch4 = qCos(phase * 0.7f) * 1.5f;

    const std::array<float, kChannels> s { ch1, ch2, ch3, ch4};
    m_ring.push(s);

    const float trigSample = s[m_trigger.channel];
    if(!m_renderEnabled && m_trigger.update(trigSample)) {
        m_renderEnabled = true;
    }
    emit frameReady();
}

void DataSource::setTriggerChannel(int channel) {
    if(channel < 0) channel = 0;
    if(channel >= kChannels) channel = kChannels - 1;

    m_trigger.channel = channel;
    armTrigger();
}
