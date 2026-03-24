import QtQuick

Item {
    id: root
    property alias text: label.text
    property bool checkable: true
    property bool checked: false
    property bool enabled: true

    property color accentColor: "#3daee9"
    property color uncheckedColor: "#3c3c3c"
    property color checkedColor: accentColor
    property color borderUncheckedColor: "#6c6c6c"
    property color borderCheckedColor: Qt.lighter(checkedColor, 1.25)

    property color textUncheckedColor: "white"
    property color textCheckedColor: "black"

    property real pressedScale: 0.98

    signal toggled(bool checked)
    signal clicked()

    implicitWidth: label.implicitWidth + 20
    implicitHeight: label.implicitHeight + 14

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 8
        opacity: root.enabled ? 1.0 : 0.5

        color: root.checked ? root.checkedColor : root.uncheckedColor
        border.width: 2
        border.color: root.checked ? root.borderCheckedColor : root.borderUncheckedColor

        Behavior on color { ColorAnimation { duration: 90 } }
        Behavior on scale { NumberAnimation { duration: 60 } }
        //color: root.enabled ? (root.checked ? "#2e7d32" : "#c62626") : "#555555"
    }

    Text {
        id: label
        anchors.centerIn: parent
        color: root.checked ? root.textCheckedColor : root.textUncheckedColor
        font.pointSize: 18
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled

        onPressed: bg.scale = root.pressedScale
        onReleased: bg.scale = 1.0
        onCanceled: bg.scale = 1.0

        onClicked: {
            root.clicked()
            if(root.checkable) {
                root.checked = !root.checked
                root.toggled(root.checked)
            }
        }
    }
}
