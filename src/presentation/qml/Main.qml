/**
 * @file Main.qml
 * @brief Defines the operator dashboard for live equipment telemetry.
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 1180
    height: 760
    minimumWidth: 940
    minimumHeight: 640
    visible: true
    title: "Equipment Monitor"
    color: "#0b1120"

    required property var viewModel

    readonly property color panelColor: "#111a2d"
    readonly property color panelBorder: "#22304a"
    readonly property color primaryText: "#edf4ff"
    readonly property color secondaryText: "#8fa2c2"
    readonly property color cyan: "#32d6c9"
    readonly property color warning: "#ffb648"
    readonly property color danger: "#ff5d73"

    component MetricCard: Rectangle {
        id: metricCard

        property string label: ""
        property string value: ""
        property string caption: ""
        property color accent: root.cyan

        radius: 14
        color: root.panelColor
        border.color: root.panelBorder
        border.width: 1
        implicitHeight: 126

        Rectangle {
            width: 4
            height: 52
            radius: 2
            color: parent.accent
            anchors.left: parent.left
            anchors.leftMargin: 18
            anchors.verticalCenter: parent.verticalCenter
        }

        Column {
            anchors.left: parent.left
            anchors.leftMargin: 38
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            spacing: 7

            Text {
                text: metricCard.label.toUpperCase()
                color: root.secondaryText
                font.pixelSize: 11
                font.weight: Font.DemiBold
                font.letterSpacing: 1.1
            }

            Text {
                text: metricCard.value
                color: root.primaryText
                font.pixelSize: 26
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                width: parent.width
            }

            Text {
                text: metricCard.caption
                color: root.secondaryText
                font.pixelSize: 12
                elide: Text.ElideRight
                width: parent.width
            }
        }
    }

    header: Rectangle {
        height: 72
        color: "#0e1728"
        border.color: root.panelBorder

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 26
            anchors.rightMargin: 26
            spacing: 16

            Rectangle {
                Layout.preferredWidth: 38
                Layout.preferredHeight: 38
                radius: 10
                color: "#173e49"

                Text {
                    anchors.centerIn: parent
                    text: "EM"
                    color: root.cyan
                    font.pixelSize: 13
                    font.bold: true
                }
            }

            ColumnLayout {
                spacing: 1

                Text {
                    text: "EQUIPMENT MONITOR"
                    color: root.primaryText
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1.2
                }

                Text {
                    text: "Industrial telemetry dashboard"
                    color: root.secondaryText
                    font.pixelSize: 12
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                implicitWidth: statusRow.implicitWidth + 26
                implicitHeight: 36
                radius: 18
                color: root.viewModel.online ? "#143a38" : "#2b3343"
                border.color: root.viewModel.online ? "#245e59" : "#435069"

                Row {
                    id: statusRow
                    anchors.centerIn: parent
                    spacing: 9

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: root.viewModel.online ? root.cyan : root.secondaryText
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: root.viewModel.statusText
                        color: root.viewModel.online ? root.cyan : root.secondaryText
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                    }
                }
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 18

            Item { implicitHeight: 4 }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                spacing: 14

                MetricCard {
                    Layout.fillWidth: true
                    label: "Device"
                    value: root.viewModel.deviceName
                    caption: root.viewModel.deviceType
                }

                MetricCard {
                    Layout.fillWidth: true
                    label: "Temperature"
                    value: root.viewModel.sampleCount > 0
                           ? root.viewModel.currentTemperature.toFixed(1) + " °C"
                           : "—"
                    caption: "Threshold: " + root.viewModel.alarmThreshold.toFixed(0) + " °C"
                    accent: root.viewModel.alarmActive ? root.danger : root.cyan
                }

                MetricCard {
                    Layout.fillWidth: true
                    label: "Samples received"
                    value: root.viewModel.sampleCount.toString()
                    caption: "Last update: " + root.viewModel.lastUpdate
                    accent: "#6f8cff"
                }

                MetricCard {
                    Layout.fillWidth: true
                    label: "Status"
                    value: root.viewModel.alarmActive ? "Alarm" : "Normal"
                    caption: root.viewModel.alarmAcknowledged ? "Acknowledged" : "Monitoring active"
                    accent: root.viewModel.alarmActive ? root.danger : "#58d68d"
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 24
                Layout.rightMargin: 24
                Layout.bottomMargin: 24
                spacing: 18

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 720
                    Layout.preferredHeight: 440
                    radius: 16
                    color: root.panelColor
                    border.color: root.panelBorder

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true

                            ColumnLayout {
                                spacing: 3

                                Text {
                                    text: "REAL-TIME TEMPERATURE"
                                    color: root.primaryText
                                    font.pixelSize: 13
                                    font.weight: Font.DemiBold
                                    font.letterSpacing: 0.8
                                }

                                Text {
                                    text: "Latest " + root.viewModel.temperatureSeries.length + " samples"
                                    color: root.secondaryText
                                    font.pixelSize: 12
                                }
                            }

                            Item { Layout.fillWidth: true }

                            Rectangle {
                                Layout.preferredWidth: 10
                                Layout.preferredHeight: 10
                                radius: 5
                                color: root.warning
                            }

                            Text {
                                text: "Threshold"
                                color: root.secondaryText
                                font.pixelSize: 12
                            }

                            Rectangle {
                                Layout.preferredWidth: 10
                                Layout.preferredHeight: 10
                                radius: 5
                                color: root.cyan
                            }

                            Text {
                                text: "Value"
                                color: root.secondaryText
                                font.pixelSize: 12
                            }
                        }

                        Canvas {
                            id: telemetryChart
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            property real chartMinimum: 40
                            property real chartMaximum: 120

                            onWidthChanged: requestPaint()
                            onHeightChanged: requestPaint()

                            Connections {
                                target: root.viewModel

                                function onTelemetryChanged() {
                                    telemetryChart.requestPaint()
                                }

                                function onThresholdChanged() {
                                    telemetryChart.requestPaint()
                                }
                            }

                            onPaint: {
                                const ctx = getContext("2d")
                                const values = root.viewModel.temperatureSeries
                                const left = 42
                                const right = width - 12
                                const top = 10
                                const bottom = height - 28
                                const plotWidth = Math.max(1, right - left)
                                const plotHeight = Math.max(1, bottom - top)

                                ctx.clearRect(0, 0, width, height)
                                ctx.lineWidth = 1
                                ctx.font = "11px Segoe UI"

                                for (let gridValue = 40; gridValue <= 120; gridValue += 20) {
                                    const gridY = bottom - ((gridValue - chartMinimum)
                                                        / (chartMaximum - chartMinimum)) * plotHeight
                                    ctx.beginPath()
                                    ctx.strokeStyle = "#26354f"
                                    ctx.moveTo(left, gridY)
                                    ctx.lineTo(right, gridY)
                                    ctx.stroke()

                                    ctx.fillStyle = "#778ba9"
                                    ctx.fillText(gridValue.toString(), 8, gridY + 4)
                                }

                                const thresholdY = bottom - ((root.viewModel.alarmThreshold - chartMinimum)
                                                           / (chartMaximum - chartMinimum)) * plotHeight
                                ctx.beginPath()
                                ctx.strokeStyle = root.warning
                                ctx.lineWidth = 1.5
                                ctx.moveTo(left, thresholdY)
                                ctx.lineTo(right, thresholdY)
                                ctx.stroke()

                                if (values.length < 2)
                                    return

                                ctx.beginPath()
                                ctx.strokeStyle = root.cyan
                                ctx.lineWidth = 2.5

                                for (let index = 0; index < values.length; ++index) {
                                    const x = left + (index / Math.max(1, values.length - 1)) * plotWidth
                                    const bounded = Math.max(chartMinimum,
                                                             Math.min(chartMaximum, Number(values[index])))
                                    const y = bottom - ((bounded - chartMinimum)
                                                      / (chartMaximum - chartMinimum)) * plotHeight
                                    if (index === 0)
                                        ctx.moveTo(x, y)
                                    else
                                        ctx.lineTo(x, y)
                                }

                                ctx.stroke()
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: 330
                    Layout.fillHeight: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 230
                        radius: 16
                        color: root.viewModel.alarmActive ? "#291724" : root.panelColor
                        border.color: root.viewModel.alarmActive ? "#763044" : root.panelBorder

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 12

                            RowLayout {
                                Layout.fillWidth: true

                                Rectangle {
                                    Layout.preferredWidth: 12
                                    Layout.preferredHeight: 12
                                    radius: 6
                                    color: root.viewModel.alarmActive ? root.danger : "#58d68d"
                                }

                                Text {
                                    text: root.viewModel.alarmActive ? "ACTIVE ALARM" : "NO ACTIVE ALARMS"
                                    color: root.viewModel.alarmActive ? root.danger : "#58d68d"
                                    font.pixelSize: 12
                                    font.weight: Font.Bold
                                    font.letterSpacing: 0.7
                                }

                                Item { Layout.fillWidth: true }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: root.viewModel.alarmMessage
                                color: root.primaryText
                                font.pixelSize: 17
                                font.weight: Font.Medium
                                wrapMode: Text.WordWrap
                            }

                            Item { Layout.fillHeight: true }

                            Button {
                                Layout.fillWidth: true
                                text: root.viewModel.alarmAcknowledged
                                      ? "Acknowledged"
                                      : "Acknowledge alarm"
                                enabled: root.viewModel.alarmActive
                                         && !root.viewModel.alarmAcknowledged
                                onClicked: root.viewModel.acknowledgeAlarm()
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 16
                        color: root.panelColor
                        border.color: root.panelBorder

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 20
                            spacing: 10

                            Text {
                                text: "CONTROL"
                                color: root.primaryText
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                                font.letterSpacing: 0.8
                            }

                            Text {
                                text: "Alarm threshold, °C"
                                color: root.secondaryText
                                font.pixelSize: 12
                            }

                            SpinBox {
                                Layout.fillWidth: true
                                from: 40
                                to: 120
                                editable: true
                                value: Math.round(root.viewModel.alarmThreshold)
                                onValueModified: root.viewModel.alarmThreshold = value
                            }

                            Item { Layout.fillHeight: true }

                            Button {
                                Layout.fillWidth: true
                                text: root.viewModel.online ? "Stop monitoring" : "Start monitoring"
                                onClicked: {
                                    if (root.viewModel.online)
                                        root.viewModel.stopMonitoring()
                                    else
                                        root.viewModel.startMonitoring()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
