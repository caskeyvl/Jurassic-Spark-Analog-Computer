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

class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(QQuickView *appViewer, QObject *parent = nullptr);
    Q_INVOKABLE void setTriggerLevel(float level);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setSamplesPerView(int n);

public slots:
    //void generateData(int type, int rowCount, int colCount);
    void update(QAbstractSeries *series);

private:
    void checkTrigger(float current);

    QQuickView *m_appViewer = nullptr;
    QTimer m_sampleTimer;
    QVector<float> m_buffer;

    int m_writeIndex = 0;
    float m_prevSample = 0.0f;
    float m_triggerLevel = 0.0f;
    bool m_armed = true;
    int m_triggerIndex = -1;
    int m_samplesPerView = 1024;
    bool m_triggered = false;
    int m_preTrigger = 250;
    bool m_renderEnabled = false;

    // QList<QList<QPointF>> m_data;

private slots:
    void sample();

signals:
    void frameReady();
};

#endif
