import QtQuick
import QtQuick.Controls.Basic

Item{
    id: root
    property alias from: slider.from
    property alias to: slider.to
    property alias stepSize: slider.stepSize
    property real value: slider.value

    implicitHeight: 60
    implicitWidth: 240

    function snap(v) {
        const s = root.stepSize
        if (s <= 0) return v

        const f = slider.from
        const n = Math.round((v-f) / s)
        const snapped = f + n * s
        return Math.max(slider.from, Math.min(slider.to, snapped))
    }
    Component.onCompleted: {
        slider.value = root.value
    }

    Slider {
        id: slider
        from: root.from
        to: root.to
        stepSize: 0

        anchors.fill: parent
        snapMode: Slider.SnapAlways

        Component.onCompleted: slider.value = root.value

        // Dragging snap
        onMoved: {
            const snapped = root.snap(value)
            if (snapped !== value)
                value = snapped
        }

        onValueChanged: {
            root.value = value
            const snapped = root.snap(value)
            if(snapped !== value)
                value = snapped
        }

        // background
        background: Item {
            id: track
            x: slider.leftPadding
            width: slider.availableWidth
            height: 40
            y: (slider.height - height) / 2

            Rectangle {
                id: base
                anchors.fill: parent
                //radius: height / 2
                color: "#d7d9c2"
            }

            Rectangle {
                id: fill
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                width: Math.max(0, slider.visualPosition * parent.width)
                //radius: Math.min(base.radius, width / 2)
                color: "#21be2b"
                z: 1
            }
        }

        // Slider
        handle: Rectangle {
            width: 4
            height: track.height + 10
            radius: 2

            x: track.x + slider.visualPosition * track.width - width / 2
            y: track.y + (track.height - height) / 2

            color: slider.pressed ? "#f0f0f0" : "#f6f6f6"
            border.color: "#bdbebf"
            border.width: 2
        }

    }

}
