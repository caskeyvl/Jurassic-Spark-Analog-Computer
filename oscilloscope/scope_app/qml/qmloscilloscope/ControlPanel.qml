// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

RowLayout {
    property alias openGLButton: openGLButton
    property alias antialiasButton: antialiasButton

    signal seriesTypeChanged(string type)
    signal refreshRateChanged(variant rate);
    signal signalSourceChanged(string source, int signalCount, int sampleCount);
    signal antialiasingEnabled(bool enabled)
    signal openGlChanged(bool enabled)
    signal settingsRequested()

    spacing: 0

    Text {
        text: "Scope"
        font.pointSize: 18
        color: "white"
    }

    MultiButton {
        id: openGLButton
        text: "OpenGL: "
        items: ["false", "true"]
        currentSelection: openGLSupported ? 1 : 0
        onSelectionChanged: openGlChanged(currentSelection == 1);
        enabled: openGLSupported
        Layout.fillHeight: true

    }

    MultiButton {
        text: "Graph: "
        items: ["line", "scatter"]
        currentSelection: 0
        onSelectionChanged: seriesTypeChanged(items[currentSelection]);
        Layout.fillHeight: true

    }

    MultiButton {
        id: signalSourceButton
        text: "Source: "
        items: ["sin", "linear", "square"]
        currentSelection: 0
        Layout.fillHeight: true
        onSelectionChanged: selection => signalSourceChanged(
                                selection,
                                5,
                                sampleCountButton.items[sampleCountButton.currentSelection]);
    }

    MultiButton {
        id: sampleCountButton
        text: "Samples: "
        items: ["6", "128", "1024", "10000"]
        currentSelection: 2
        Layout.fillHeight: true
        onSelectionChanged: selection => signalSourceChanged(
                                signalSourceButton.items[signalSourceButton.currentSelection],
                                5,
                                selection);
    }

    MultiButton {
        text: "Refresh rate: "
        items: ["1", "24", "60"]
        currentSelection: 2
        onSelectionChanged: refreshRateChanged(items[currentSelection]);
        Layout.fillHeight: true
    }

    MultiButton {
        id: antialiasButton
        text: "Antialias: "
        items: ["OFF", "ON"]
        enabled: true
        currentSelection: 0
        onSelectionChanged: antialiasingEnabled(currentSelection == 1);
        Layout.fillHeight: true
    }

    ToggleButton {
        id: settingsButton
        text: "Settings"
        checked: true
        onClicked: settingsRequested()
        Layout.fillHeight: true
        width: 200
    }
}
