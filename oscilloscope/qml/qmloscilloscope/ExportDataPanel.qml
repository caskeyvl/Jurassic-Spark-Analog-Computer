import QtQuick

Item {
    id: root
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
            text: "Export Data"
            color: "white"
            font.pointSize: 30
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Text {
            text: "Note: Oscilloscope must be paused."
            color: "white" 
            font.pointSize: 16
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Rectangle { height: 1; width: parent.width; color: "#444" }

        ToggleButton {
            text: "test file"
            checkable: false
            checked: true
            textCheckedColor: "black"
            width: parent.width
            height: 80
            onClicked: dataSource.exportCsv("/tmp/scope_export.csv")

        }

    }

    ToggleButton {
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
}
