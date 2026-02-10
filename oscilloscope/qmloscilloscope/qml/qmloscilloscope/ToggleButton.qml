import QtQuick

Item {
    id: root
    property string text: "Toggle"
    property bool checked: false
    signal toggled(bool checked)
    signal clicked()

    implicitWidth: label.implicitWidth + 24
    implicitHeight: label.implicitHeight + 14

    Rectangle {
        anchors.fill: parent
        radius: 4
        border.color: "#333"

        color: root.enabled ? (root.checked ? "#2e7d32" : "#c62626") : "#555555"
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: "white"
        font.pointSize: 18
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        onClicked: {
            root.clicked()
            root.checked = !root.checked
            root.toggled(root.checked)
        }
    }
}
