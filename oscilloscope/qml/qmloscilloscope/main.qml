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

        onSettingsRequested: settingsDrawer.toggle()
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
        enabled: !triggerSettingsDrawer.open && !axisSettingsDrawer.open && !exportDataDrawer.open
        onCloseRequested: hide()
        onTestToggled: on => console.log("test toggled:", on)
        onTriggerSettingsRequested: triggerSettingsDrawer.toggle()
        onAxisSettingsRequested: axisSettingsDrawer.toggle()
        onExportDataRequested: exportDataDrawer.toggle()
    }

    TriggerSettingsPanel {
        id: triggerSettingsDrawer
        z: 100
        modal: true
        onCloseRequested: hide()
    }

    AxisSettingsPanel {
        id: axisSettingsDrawer
        z: 100
        modal: true
        onCloseRequested: hide()
        onTimeRangeChanged: seconds => scopeView.setTimeRange(seconds)
        onChannelRangeChanged: (channel, range) => scopeView.setChannelAxisRange(channel, range)
    }

    ExportDataPanel {
        id: exportDataDrawer
        z: 100
        modal: true
        scopeViewRef: scopeView
        onCloseRequested: hide()
    }

    SplashScreen {
        anchors.fill: parent
    }

    Component.onCompleted: {
        scopeView.redraw()
    }
}
