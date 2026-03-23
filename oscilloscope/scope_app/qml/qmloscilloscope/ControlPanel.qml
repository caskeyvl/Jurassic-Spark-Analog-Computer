// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

RowLayout {
    //property alias openGLButton: openGLButton
    //property alias antialiasButton: antialiasButton
    id: root
    signal seriesTypeChanged(string type)
    signal refreshRateChanged(variant rate);
    signal signalSourceChanged(string source, int signalCount, int sampleCount);
    signal antialiasingEnabled(bool enabled)
    signal openGlChanged(bool enabled)
    signal settingsRequested()
    signal channelToggle(int channel, bool enabled)

    spacing: 0

    Text {
        text: "Scope"
        font.pointSize: 18
        color: "white"
    }

    ToggleButton {
        id: channel1Button
        text: "Channel 1"
        accentColor: "#FFD54A"
        Layout.fillHeight: true
        Layout.preferredWidth: 300
        onToggled: (isChecked) => root.channelToggle(1, isChecked)
    }

    ToggleButton {
        id: channel2Button
        text: "Channel 2"
        accentColor: "#4DD0E1"
        Layout.fillHeight: true
        Layout.preferredWidth: 300
        onToggled: (isChecked) => root.channelToggle(2, isChecked)

    }

    ToggleButton {
        id: channel3Button
        text: "Channel 3"
        accentColor: "#A5D6A7"
        Layout.fillHeight: true
        Layout.preferredWidth: 300
        onToggled: (isChecked) => root.channelToggle(3, isChecked)
    }

    ToggleButton {
        id: channel4Button
        text: "Channel 4"
        accentColor: "#CE93D8"
        Layout.fillHeight: true
        Layout.preferredWidth: 300
        onToggled: (isChecked) => root.channelToggle(4, isChecked)
    }

    ToggleButton {
        id: startStopButton

        checked: true
        text: checked ? "Stop" : "Start"

        checkedColor: "#c62626"
        uncheckedColor: "#2e7d32"
        textCheckedColor: "white"
        textUncheckedColor: "white"

        Layout.fillHeight: true
        Layout.preferredWidth: 100

        onToggled: checked ? dataSource.start() : dataSource.stop()
    }

    ToggleButton {
        id: settingsButton
        text: "Settings"
        checkable: false

        checked: true
        textCheckedColor: "black"
        accentColor: "#cccccc"

        Layout.fillHeight: true
        Layout.preferredWidth: 175

        onClicked: settingsRequested()
    }
}
