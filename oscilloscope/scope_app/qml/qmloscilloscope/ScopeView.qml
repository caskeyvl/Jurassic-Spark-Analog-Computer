// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtCharts

//![1]
Item {
    id: root

    property bool openGl: openGLSupported
    property bool running: false
    property int samplesPerView: 1024

    property int refreshHz: 60
    property var series1: null
    property var series2: null

    ChartView {
        id: chartView
        anchors.fill: parent
        antialiasing: true
        legend.visible: false
        animationOptions: ChartView.NoAnimation
        theme: ChartView.ChartThemeDark

        ValueAxis { id: axisY1; min: -1; max: 4}
        ValueAxis { id: axisY2; min: -10; max: 5}
        ValueAxis { id: axisX; min: 0; max: root.samplesPerView }

        LineSeries {
            id: channel1
            name: "Channel 1"
            axisX: axisX
            axisY: axisY1
            useOpenGL: root.openGl
        }
        LineSeries {
            id: channel2
            name: "Channel 2"
            axisX: axisX
            axisY: axisY2
            useOpenGL: root.openGl
        }
    }

    Timer {
        id: refreshTimer
        interval: 1000 / root.refreshHz
        running: root.running
        repeat: true
        onTriggered: root.redraw()
    }

    onSamplesPerViewChanged: {
        axisX.max = root.samplesPerView - 1
        root.redraw()
    }

    Component.onCompleted: {
        root.series1 = channel1
        root.series2 = channel2
    }

    function redraw() {
        dataSource.setSamplesPerView(samplesPerView)
        dataSource.update(channel1)
        dataSource.update(channel2)
    }

    function changeRefreshRate (rate) {
        refreshTimer.interval = 1000 / Number(rate)
    }

    function changeSeriesType(type) {
        chartView.removeAllSeries()

        var s1, s2
        if(type === "linear") {
            s1 = chartView.createSeries(ChartView.SeriesTypeLine, "Channel 1", axisX, axisY1)
            s2 = chartView.createSeries(ChartView.SeriesTypeLine, "Channel 2", axisX, axisY2)
        } else {
            s1 = chartView.createSeries(ChartView.SeriesTypeScatter, "Channel1", axisX, axisY1)
            s1.markerSize = 2
            s1.borderColor = "transparent"

            s2 = chartView.createSeries(ChartView.SeriesTypeScatter, "Channel2", axisX, axisY2)
            s2.markerSize = 2
            s2.borderColor = "transparent"
        }
        s1.useOpenGL= root.openGl
        s2.useOpenGL= root.openGl
    }

    function setSamplesPerView(n) {
        root.samplesPerView = Number(n)
        axisX.max = root.samplesPerView
    }
}
