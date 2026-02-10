import QtQuick

Item {
    id: root

    // API
    property bool open: false
    property real drawerWidth: Math.min(parent ? parent.width * 0.35: 420, 420)
    property bool modal: false
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
    x: (parent ? parent.width : 0) - (open ? width : 0)
    Behavior on x {
        NumberAnimation { duration: 220; easing.type: Easing.InOutCubic }
    }

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
            font.pointSize: 18
            anchors.horizontalCenter: parent.horizontalCenter
        }

        ToggleButton {
            text: "Test"
            checked: true
            anchors.horizontalCenter: parent.horizontalCenter
            onToggled: root.testToggled(checked)
        }

        Rectangle { height: 1; width: parent.width; color: "#444" }

        ToggleButton {
            text: "Close"
            checked: true
            anchors.horizontalCenter: parent.horizontalCenter
            onToggled: root.closeRequested()
        }
    }
}

