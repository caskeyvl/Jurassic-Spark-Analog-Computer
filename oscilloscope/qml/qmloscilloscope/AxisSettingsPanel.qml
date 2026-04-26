import QtQuick

Item {
    id: root

    // API
    property bool open: false
    property bool ready: false
    property real drawerWidth: Math.min(parent ? parent.width * 0.35: 420, 420)
    property bool modal: true
    property bool closeOnScrim: false

    signal closeRequested()
    signal timeRangeChanged(real seconds)
    signal channelRangeChanged(int channel, real range)

    function show() { open = true }
    function hide() { open = false }
    function toggle() { open = !open }

anchors.top: parent ? parent.top : undefined
    anchors.bottom: parent ? parent.bottom : undefined

    width: drawerWidth
    height: parent ? parent.height : 0

    // Slide animation
    x: parent ? (parent.width - (open ? width : 0)) : 0
    Behavior on x {
        enabled: ready
        NumberAnimation { duration: 1000; easing.type: Easing.InOutQuart }
    }

    Rectangle {
        anchors.fill: parent
        color: "#2b2b2b"
        border.color: "#444444"
        radius: 6
    }

    default property alias content: contentItem.data

    Item {
        id: contentItem
        anchors.fill: parent
        anchors.margins: 12
    }

    Text {
        id: panelTitle
        text: "Axis Settings"
        color: "white"
        font.pointSize: 30
        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Rectangle {
        id: titleDivider
        anchors.top: panelTitle.bottom
        anchors.topMargin: 8
        height: 1
        width: parent.width
        color: "#444"
    }

    ToggleButton {
        id: backButton
        text: "Back"
        checkable: false
        checked: true
        checkedColor: "#c62626"
        textCheckedColor: "white"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        width: parent.width - 24
        height: 150
        onClicked: root.closeRequested()
    }

    Flickable {
        anchors.top: titleDivider.bottom
        anchors.bottom: backButton.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        contentHeight: scrollContent.implicitHeight
        clip: true

        Column {
            id: scrollContent
            width: parent.width
            spacing: 8

            Text {
                text: "Time Range"
                color: "white"
                font.pointSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            SnapperSlider {
                id: timeRangeSlider
                from: 0.1
                to: 10
                value: 5
                stepSize: 0.1
                width: parent.width
                onValueChanged: root.timeRangeChanged(value)
            }

            Text {
                text: "Window: " + timeRangeSlider.value.toFixed(1) + " s"
                color: "white"
                font.pixelSize: 18
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Rectangle { height: 1; width: parent.width; color: "#444" }

            Text {
                text: "Y Scale"
                color: "white"
                font.pointSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text { text: "Channel 1"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
            SnapperSlider {
                id: ch1AxisSlider
                from: 0.5; to: 15; value: 10; stepSize: 0.5
                width: parent.width
                onValueChanged: root.channelRangeChanged(0, value)
            }
            Text { text: "+- " + ch1AxisSlider.value.toFixed(1) + " V"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }

            Text { text: "Channel 2"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
            SnapperSlider {
                id: ch2AxisSlider
                from: 0.5; to: 15; value: 10; stepSize: 0.5
                width: parent.width
                onValueChanged: root.channelRangeChanged(1, value)
            }
            Text { text: "+- " + ch2AxisSlider.value.toFixed(1) + " V"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }

            Text { text: "Channel 3"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
            SnapperSlider {
                id: ch3AxisSlider
                from: 0.5; to: 15; value: 10; stepSize: 0.5
                width: parent.width
                onValueChanged: root.channelRangeChanged(2, value)
            }
            Text { text: "+- " + ch3AxisSlider.value.toFixed(1) + " V"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }

            Text { text: "Channel 4"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
            SnapperSlider {
                id: ch4AxisSlider
                from: 0.5; to: 15; value: 10; stepSize: 0.5
                width: parent.width
                onValueChanged: root.channelRangeChanged(3, value)
            }
            Text { text: "+- " + ch4AxisSlider.value.toFixed(1) + " V"; color: "white"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }

            Rectangle { height: 1; width: parent.width; color: "#444" }

            Text {
                text: "Noise Filter"
                color: "white"
                font.pointSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Slider value is "smoothing" 0–1; alpha = 1 - smoothing * 0.97
            // so the range is alpha 1.0 (off) → 0.03 (heavy).
            SnapperSlider {
                id: filterSlider
                from: 0; to: 1; value: 0; stepSize: 0.01
                width: parent.width
                onValueChanged: dataSource.setFilterAlpha(1.0 - value * 0.97)
            }
            Text {
                text: filterSlider.value === 0
                      ? "Off"
                      : filterSlider.value < 0.35 ? "Light (" + (filterSlider.value * 100).toFixed(0) + "%)"
                      : filterSlider.value < 0.7  ? "Medium (" + (filterSlider.value * 100).toFixed(0) + "%)"
                      : "Heavy (" + (filterSlider.value * 100).toFixed(0) + "%)"
                color: "white"
                font.pixelSize: 18
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
