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

DataSource::DataSource(QQuickView *appViewer, QObject *parent)
    : QObject(parent)
    , m_appViewer(appViewer)
{
    m_buffer.resize(8192);

    connect(&m_sampleTimer, &QTimer::timeout, this, &DataSource::sample);
    m_sampleTimer.start(1);
}

void DataSource::update(QAbstractSeries *series)
{
    if (!m_renderEnabled) return;

    auto *xy = qobject_cast<QXYSeries *>(series);
    if(!xy) return;

    // sample count
    const int size = m_buffer.size();
    const int N = qMin(m_samplesPerView, size);
    if(N <= 1) return;

    QList<QPointF> points;
    points.reserve(N);

    // m_writeIndex points to next sample, so current sample is at m_writeIndex - 1
    int end = m_writeIndex - 1;
    if (end < 0) end += size;
    int start = end - (N - 1);
    start %= size;
    if (start < 0) start += size;

    for (int i = 0; i < N; i++) {
        int idx = start + i;
        if (idx >= size) idx -= size;
        points.append(QPointF(i, m_buffer[idx]));
    }

    xy->replace(points);
    //emit frameReady();
}

void DataSource::sample() {
    // produce one sample and write it in ring buffer

    //float t = m_writeIndex * 0.05f;
    static float phase = -4.0f;
    phase += 0.02f;
    if (phase > 8.0f) phase = -1.0f;
    float y = phase;
    //float y = qSin(M_PI / 50 * t) + 0.5 + QRandomGenerator::global()->generateDouble();

    m_buffer[m_writeIndex] = y;

    checkTrigger(y);

    m_writeIndex = (m_writeIndex + 1) % m_buffer.size();

   //emit frameReady();
}

void DataSource::setTriggerLevel(float level) {
    m_triggerLevel = level;
    m_armed = true;         // re arm
    m_renderEnabled = false;
}

void DataSource::checkTrigger(float current) {
    if (!m_renderEnabled && m_prevSample < m_triggerLevel && current >= m_triggerLevel) {
        m_renderEnabled = true;
        m_armed = false;
    }
    m_prevSample = current;
}

void DataSource::setSamplesPerView(int n) {
    if (n < 16) n = 16;
    m_samplesPerView = n;
}

void DataSource::start() {
    if (!m_sampleTimer.isActive()) m_sampleTimer.start(1);
}

void DataSource::stop() {
    m_sampleTimer.stop();
}
