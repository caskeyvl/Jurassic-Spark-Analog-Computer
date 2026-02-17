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

    // qRegisterMetaType<QAbstractSeries*>();
    // qRegisterMetaType<QAbstractAxis*>();

    // generateData(0, 5, 1024);
}

void DataSource::update(QAbstractSeries *series)
{
    auto *xy = qobject_cast<QXYSeries *>(series);
    if(!xy) return;

    // sample count
    const int N = qMin(m_samplesPerView, m_buffer.size());
    if(N <= 1) return;

    QList<QPointF> points;
    points.reserve(N);

    // m_writeIndex points to next sample, so current sample is at m_writeIndex - 1
    int end = m_writeIndex - 1;
    if (end < 0)  end += m_buffer.size();

    // build points left to right
    for(int i = 0; i < N; ++i) {
        int idx = end - (N - 1 - i);
        idx %= m_buffer.size();
        if(idx < 0) idx += m_buffer.size();
        points.append(QPointF(i, m_buffer[idx]));
    }

    xy->replace(points);
}

// void DataSource::generateData(int type, int rowCount, int colCount)
// {
//     // Remove previous data
//     m_data.clear();

//     // Append the new data depending on the type
//     for (int i(0); i < rowCount; i++) {
//         QList<QPointF> points;
//         points.reserve(colCount);
//         for (int j(0); j < colCount; j++) {
//             qreal x(0);
//             qreal y(0);
//             switch (type) {
//             case 0:
//                 // data with sin + random component
//                 y = qSin(M_PI / 50 * j) + 0.5 + QRandomGenerator::global()->generateDouble();
//                 x = j;
//                 break;
//             case 1:
//                 // linear data
//                 x = j;
//                 y = (qreal) i / 10;
//                 break;
//             case 2:
//                 // square wave
//                 x = j;
//                 y = ((j % 80) < 40) ? 1 : 0;
//                 y += 0.05 * (QRandomGenerator::global()->generateDouble() - 0.5);
//             default:
//                 // unknown, do nothing
//                 break;
//             }
//             points.append(QPointF(x, y));
//         }
//         m_data.append(points);
//     }
// }

void DataSource::sample() {
    // produce one sample and write it in ring buffer

    float t = m_writeIndex * 0.05f;
    float y = qSin(M_PI / 50 * t) + 0.5 + QRandomGenerator::global()->generateDouble();

    m_buffer[m_writeIndex] = y;

    m_prevSample = y;
    m_writeIndex = (m_writeIndex + 1) % m_buffer.size();

   // emit frameReady();
}

void DataSource::setTriggerLevel(float level) {
    m_triggerLevel = level;
    m_armed = true;         // re arm
    // reset triggerIndex?
}

void DataSource::checkTrigger(float current) {
    if ((m_armed && m_prevSample < m_triggerLevel) && current >= m_triggerLevel) {
        m_triggerIndex = m_writeIndex;
        m_armed = false;
        emit frameReady();
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
