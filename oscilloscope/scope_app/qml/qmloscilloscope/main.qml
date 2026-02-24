// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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

        onSeriesTypeChanged: type => scopeView.changeSeriesType(type)
        onRefreshRateChanged: rate => scopeView.changeRefreshRate(rate)
        // onAntiAliasingEnabled: enabled => scopeViewView.antialiasing = enabled
        onOpenGlChanged: enabled => scopeView.openGl = enabled
        onSettingsRequested: settingsDrawer.toggle()

        onSignalSourceChanged: (source, signalCount, sampleCount) => {
                                   scopeView.setSamplesPerView(sampleCount)
                                   dataSource.setSignalType(source === "sin" ? 0 : source === "linear" ? 1 : 2)
                               }
        onChannelToggle: (ch, enabled) => scopeView.setChannelEnabled(ch, enabled)
    }

    ScopeView {
        id: scopeView
        anchors.top: parent.top
        anchors.bottom: controlPanel.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: main.height
    }

    SettingsPanel {
        id: settingsDrawer
        z: 100
        modal: true
        onCloseRequested: hide()
        onTestToggled: on => console.log("test toggled:", on)
    }

    Component.onCompleted: {
        scopeView.running = true
        scopeView.redraw()
        dataSource.start()
    }
}
