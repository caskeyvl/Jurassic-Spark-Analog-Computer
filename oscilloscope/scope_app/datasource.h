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
    Q_INVOKABLE void setTriggerChannel(int channel);
    Q_INVOKABLE void setSamplesPerView(int n);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    Q_INVOKABLE void updateChannel(int channel, QAbstractSeries *series);

signals:
    void frameReady();

private slots:
    void sample();

private:
    static constexpr int kChannels = 4;

    struct ScopeRing {
        std::array<QVector<float>, kChannels> ch;
        int size = 0;
        int write = 0;
        void resize(int n) {
            size = n;
            for (auto &v : ch) v.resize(n);
            write = 0;
        }

        void push(const std::array<float, kChannels>& s) {
            for(int c = 0; c < kChannels; ++c) ch[c][write] = s[c];
            write = (write + 1) % size;
        }

        int lastIndex() const {
            if (size <= 0) return 0;
            int end = write - 1;
            if (end < 0) end += size;
            return end;
        }
    };

    struct Trigger {
        int channel = 0;
        float level = 0.0f;
        bool enabled = true;
        bool armed = true;
        float prev = 0.0f;

        bool update(float current) {
            if(!enabled || !armed) { prev = current; return false; }
            bool fired = (prev < level && current >= level);
            prev = current;
            if(fired) armed = false;
            return fired;
        }

        void rearm() { armed = true; }
    };

    void armTrigger();

    QQuickView *m_appViewer = nullptr;
    QTimer m_sampleTimer;

    ScopeRing m_ring;
    Trigger m_trigger;

    int m_samplesPerView = 1024;
    bool m_renderEnabled = false;

    std::array<QVector<QPointF>, kChannels> m_scratchPoints;
};

#endif
