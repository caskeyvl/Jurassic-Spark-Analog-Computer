// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts

Item {
    id: button

    property string text: "Option: "
    property variant items: ["first"]
    property int currentSelection: 0

    signal selectionChanged(variant selection)
    signal clicked

    implicitWidth: buttonText.implicitWidth + 20
    implicitHeight: buttonText.implicitHeight + 12
    Layout.minimumWidth:1
    //Layout.fillWidth: true

    Rectangle {
        anchors.fill: parent
        radius: 3
        gradient: button.enabled ? enabledGradient : disabledGradient

        Gradient {
            id: enabledGradient
            GradientStop { position: 0.0; color: "#eeeeee" }
            GradientStop { position: 1.0; color: "#cccccc" }
        }
        Gradient {
            id: disabledGradient
            GradientStop { position: 0.0; color: "#444444" }
            GradientStop { position: 1.0; color: "#666666" }
        }

        Text {
            id: buttonText
            text: button.text + button.items[currentSelection]
            clip: true
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.centerIn: parent
            font.pointSize: 14
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                button.clicked()
                currentSelection = (currentSelection + 1) % items.length;
                selectionChanged(button.items[currentSelection]);
            }
        }
    }
}
