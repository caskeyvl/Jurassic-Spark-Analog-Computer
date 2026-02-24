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
    signal testToggled(bool on)

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

    // Scrim Rectangle (uncomment)
    // Rectangle {
    //     id: scrim
    //     visible: root.modal && root.open
    //     anchors.fill: parent ? parent : undefined
    //     parent: root.parent
    //     z: root.z - 1
    //     color: "#000000"
    //     opacity: 0.35

    //     Behavior on opacity {
    //         NumberAnimation { duration : 180; easing.type: Easing.InOutCubic }
    //     }

    //     MouseArea {
    //         anchors.fill: parent
    //         enabled: root.closeOnScrim
    //         onClicked: root.hide()
    //     }
    // }

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

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            text: "Settings"
            color: "white"
            font.pointSize: 24
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle { height: 1; width: parent.width; color: "#444" }

        Text {
            text: "Trigger Settings"
            color: "white"
            font.pointSize: 20
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MultiButton {
            id: triggerOnButton
            text: "Trigger on: "
            items: ["Ch1", "Ch2", "Ch3", "Ch4"]
            currentSelection: 0
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: 80

            onSelectionChanged: dataSource.setTriggerChannel(Number(currentSelection))
        }

        SnapperSlider {
            id: triggerSlider
            from: -12
            to: 12
            value: 8
            stepSize: 0.5
            width: parent.width

            onValueChanged: dataSource.setTriggerLevel(value)
        }

        Text {
            id: sliderValue
            text: "Trigger Level: " + triggerSlider.value.toFixed(1) + " V"
            color: "White"
            font.pixelSize: 18
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle { height: 1; color: "#444"; width: parent.width }
    }
    ToggleButton {
        text: "Close"
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
}
