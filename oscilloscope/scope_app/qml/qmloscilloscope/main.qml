// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

//![1]
Item {
    id: main
    width: 600
    height: 400

    ControlPanel {
        id: controlPanel
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        height: 200
//![1]

        onSignalSourceChanged: (source, signalCount, sampleCount) => {
            if (source === "sin")
                dataSource.generateData(0, signalCount, sampleCount);
            else if (source === "linear")
                dataSource.generateData(1, signalCount, sampleCount);
            else
                dataSource.generateData(2, signalCount, sampleCount);
            scopeView.axisX().max = sampleCount;
        }
        onSeriesTypeChanged: type => scopeView.changeSeriesType(type);
        onRefreshRateChanged: rate => scopeView.changeRefreshRate(rate);
        onAntialiasingEnabled: enabled => scopeView.antialiasing = enabled;
        onOpenGlChanged: enabled => scopeView.openGL = enabled;
        onSettingsRequested: settingsDrawer.toggle()
    }

//![2]
    ScopeView {
        id: scopeView
        anchors.top: parent.top
        anchors.bottom: controlPanel.top
        anchors.right: parent.right
        anchors.left: parent.left
        height: main.height
    }


    SettingsPanel {
        id: settingsDrawer
        z: 100
        modal: true

        onCloseRequested: hide()
        onTestToggled: on => console.log("test toggled:", on)
    }
//![2]
}
