import QtQuick

Item {
    id: root
    property bool open: false
    property bool ready: false
    property real drawerWidth: Math.min(parent ? parent.width * 0.35 : 420, 420)

    property bool modal: true
    property bool closeOnScrim: false

    signal closeRequested()

    function show() { open = true }
    function hide() { open = false }
    function toggle() { open = !open }

    anchors.top: parent ? parent.top : undefined
    anchors.bottom: parent ? parent.bottom : undefined

    width: drawerWidth
    height: parent ? parent.height : 0

    x: parent ? (parent.width - (open ? width : 0)) : 0
    Behavior on x {
        enabled: ready
        NumberAnimation { duration: 1000; easing.type: Easing.InOutQuart }
    }

    // ── State ────────────────────────────────────────────────────────────────
    property var scopeViewRef: null
    property string selectedAddress: ""
    property string selectedName: ""
    property string statusText: ""
    property string exportFilePath: ""
    property bool transferring: false
    property bool scanning: false

    ListModel { id: deviceModel }

    Connections {
        target: btExporter

        function onDeviceFound(name, address) {
            deviceModel.append({ "name": name, "address": address })
        }
        function onScanStopped() {
            scanning = false
            if (deviceModel.count === 0)
                statusText = "No devices found."
        }
        function onTransferStarted() {
            transferring = true
            statusText = "Sending\u2026"
        }
        function onTransferFinished(success, message) {
            transferring = false
            statusText = success ? "Sent successfully." : ("Failed: " + message)
        }
    }

    // ── Background + click blocker ────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: "#2b2b2b"
        border.color: "#444444"
        radius: 6
    }

    // Prevents clicks from passing through to panels underneath
    MouseArea {
        anchors.fill: parent
        onClicked: {}
    }

    // ── Content ───────────────────────────────────────────────────────────────
    Column {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: backButton.top
            margins: 12
            bottomMargin: 8
        }
        spacing: 8

        Text {
            text: "Export Data"
            color: "white"
            font.pointSize: 22
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            text: "Oscilloscope must be paused."
            color: "#aaaaaa"
            font.pointSize: 12
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle { height: 1; width: parent.width; color: "#444" }

        // Step 1
        Row {
            width: parent.width
            spacing: 6

            ToggleButton {
                text: "1. Export CSV"
                checkable: false
                checked: true
                checkedColor:     exportFilePath.endsWith(".csv") ? "#ffd54a" : (exportFilePath === "" ? "#ffd54a" : "#555555")
                textCheckedColor: exportFilePath.endsWith(".csv") ? "black"   : (exportFilePath === "" ? "black"   : "#aaaaaa")
                width: (parent.width - parent.spacing) / 2
                height: 66
                onClicked: {
                    const enabled = scopeViewRef ? scopeViewRef.channelEnabled : [true, true, true, true]
                    const ok = dataSource.exportCsv("/tmp/scope_export.csv", enabled)
                    if (ok) { exportFilePath = "/tmp/scope_export.csv"; statusText = "CSV saved." }
                    else    { statusText = "CSV export failed." }
                }
            }

            ToggleButton {
                text: "1. Export PNG"
                checkable: false
                checked: true
                checkedColor:     exportFilePath.endsWith(".png") ? "#ffd54a" : (exportFilePath === "" ? "#ffd54a" : "#555555")
                textCheckedColor: exportFilePath.endsWith(".png") ? "black"   : (exportFilePath === "" ? "black"   : "#aaaaaa")
                width: (parent.width - parent.spacing) / 2
                height: 66
                enabled: scopeViewRef !== null
                onClicked: {
                    scopeViewRef.grabToImage(function(result) {
                        const ok = result.saveToFile("/tmp/scope_export.png")
                        if (ok) { exportFilePath = "/tmp/scope_export.png"; statusText = "Screenshot saved." }
                        else    { statusText = "Screenshot failed." }
                    })
                }
            }
        }

        Rectangle { height: 1; width: parent.width; color: "#444" }

        // Step 2
        ToggleButton {
            text: scanning ? "Scanning\u2026" : "2. Scan for Devices"
            checkable: false
            checked: true
            textCheckedColor: "black"
            width: parent.width
            height: 66
            enabled: !transferring
            onClicked: {
                deviceModel.clear()
                selectedAddress = ""
                selectedName = ""
                statusText = ""
                scanning = true
                btExporter.startScan()
            }
        }

        // Live device list
        Rectangle {
            width: parent.width
            height: 180
            color: "#1e1e1e"
            border.color: "#444"
            radius: 4
            visible: deviceModel.count > 0

            ListView {
                anchors.fill: parent
                anchors.margins: 4
                model: deviceModel
                clip: true
                spacing: 2

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 44
                    color: model.address === selectedAddress ? "#4a90d9" : "#333333"
                    radius: 3

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        spacing: 2
                        Text { text: model.name;    color: "white";   font.pointSize: 11; font.bold: true }
                        Text { text: model.address; color: "#aaaaaa"; font.pointSize: 9 }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            selectedAddress = model.address
                            selectedName    = model.name
                            statusText      = "Selected: " + model.name
                        }
                    }
                }
            }
        }

        // Step 3
        ToggleButton {
            text: transferring ? "Sending\u2026" : "3. Send to " + (selectedName || "device")
            checkable: false
            checked: true
            textCheckedColor: selectedAddress !== "" && exportFilePath !== "" && !transferring ? "black" : "#888888"
            checkedColor:     selectedAddress !== "" && exportFilePath !== "" && !transferring ? "#ffd54a" : "#555555"
            width: parent.width
            height: 66
            enabled: selectedAddress !== "" && exportFilePath !== "" && !transferring
            onClicked: btExporter.sendFile(selectedAddress, exportFilePath)
        }

        // Status
        Text {
            text: statusText
            color: statusText.startsWith("Failed") ? "#ef5350" : "#81c784"
            font.pointSize: 11
            wrapMode: Text.WordWrap
            width: parent.width
            visible: statusText !== ""
        }
    }

    // ── Back button ───────────────────────────────────────────────────────────
    ToggleButton {
        id: backButton
        text: "Back"
        checkable: false
        checked: true
        checkedColor: "#c62626"
        textCheckedColor: "white"
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 12
        }
        width: parent.width - 24
        height: 100
        onClicked: root.closeRequested()
    }
}
