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

    function _toIndex(ch) { return Number(ch) - 1}

    property var channelSeries: [null, null, null, null]
    property var channelEnabled: [false, false, false, false]

    property int refreshHz: 60

    ChartView {
        id: chartView
        anchors.fill: parent
        antialiasing: true
        legend.visible: false
        animationOptions: ChartView.NoAnimation
        theme: ChartView.ChartThemeDark

        ValueAxis { id: axisY1; min: -10; max: 10}
        ValueAxis { id: axisY2; min: -10; max: 10}
        ValueAxis { id: axisY3; min: -10; max: 10}
        ValueAxis { id: axisY4; min: -10; max: 10}
        ValueAxis { id: axisX; min: 0; max: root.samplesPerView - 1}

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
        LineSeries {
            id: channel3
            name: "Channel 3"
            axisX: axisX
            axisY: axisY3
            useOpenGL: root.openGl
        }
        LineSeries {
            id: channel4
            name: "Channel 4"
            axisX: axisX
            axisY: axisY4
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

    Component.onCompleted: {
        root.channelSeries = [channel1, channel2, channel3, channel4]
        dataSource.setSamplesPerView(root.samplesPerView);
    }

    onSamplesPerViewChanged: {
        axisX.max = root.samplesPerView - 1
        dataSource.setSamplesPerView(root.samplesPerView)
        root.redraw()
    }

    function redraw() {
        for (var i = 0; i < 4; ++i) {
            var s = root.channelSeries[i]
            if (root.channelEnabled[i] && s) {
                dataSource.updateChannel(i, s)
            } else if (s) {
                s.clear()
            }
        }
    }

    function changeRefreshRate (rate) {
        refreshTimer.interval = 1000 / Number(rate)
    }

    function changeSeriesType(type) {
        chartView.removeAllSeries()

        var newSeries = [null, null, null, null]
        var axesY = [axisY1, axisY2, axisY3, axisY4]
        var names = ["Channel1", "Channel2", "Channel3", "Channel4"]

        for (var i = 0; i < 4; ++i) {
            var s
            if(type === "linear") {
                s = chartView.createSeries(ChartView.SeriesTypeLine, names[i], axisX, axesY[i])
            } else {
                s = chartView.createSeries(ChartView.SeriesTypeScatter, names[i], axisX, axesY[i])
                s.markerSize = 2
                s.borderColor = "transparent"
            }

            s.useOpenGL = root.openGl
            s.visible = root.channelEnabled[i]
            newSeries[i] = s
        }

        root.channelSeries = newSeries
        root.redraw()
    }

    function setSamplesPerView(n) {
        root.samplesPerView = Number(n)
        //axisX.max = root.samplesPerView
    }

    // function channelToggle(n) {
    //     if ( n === 1 ) {
    //         channel1.visible = channel1.visible ? channel1.disable : channel1.enable
    //         if (channel1Enabled) dataSource.update(ch1Series)
    //     } else if (n === 2) {
    //         channel2.visible = channel2.visible ? channel2.disable : channel2.enable
    //         if(channel2Enabled) dataSource.update(ch2Series)
    //     }
    // }

    function setChannelEnabled(ch, enabled) {
        var idx = _toIndex(ch)
        if (idx < 0 || idx > 3) return

        root.channelEnabled[idx] = enabled

        var s = root.channelSeries[idx]
        if(s) {
            s.visible = enabled
            if (!enabled) s.clear()
        }

        if (enabled) root.redraw()
    }
}
