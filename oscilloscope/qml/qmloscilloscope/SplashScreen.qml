// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: splash
    anchors.fill: parent
    color: "black"
    z: 1000

    function hide() {
        fadeOut.start()
    }

    Image {
        anchors.centerIn: parent
        source: "logo.png"
        fillMode: Image.PreserveAspectFit
        width: Math.min(parent.width * 0.5, 400)
        height: width
    }

    OpacityAnimator {
        id: fadeOut
        target: splash
        from: 1.0
        to: 0.0
        duration: 600
        onFinished: splash.visible = false
    }

    Timer {
        interval: 4000
        running: true
        repeat: false
        onTriggered: splash.hide()
    }
}
