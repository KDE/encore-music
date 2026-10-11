// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami

// Bouncing bars that show music is playing
Item {
    id: root

    property bool playing: false
    property color color: Kirigami.Theme.highlightColor

    implicitWidth: bars.implicitWidth
    implicitHeight: Kirigami.Units.iconSizes.small

    Row {
        id: bars

        height: root.height
        spacing: Math.max(1, Math.round(root.implicitHeight / 8))

        Repeater {
            model: [420, 300, 520]

            delegate: Item {
                id: bar

                required property int modelData

                width: Math.max(2, Math.round(root.implicitHeight / 5))
                height: bars.height

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: parent.height * 0.25
                    radius: width / 2
                    color: root.color

                    SequentialAnimation on height {
                        running: root.playing && root.visible
                        loops: Animation.Infinite
                        alwaysRunToEnd: true

                        NumberAnimation {
                            to: bar.height
                            duration: bar.modelData
                            easing.type: Easing.OutQuad
                        }
                        NumberAnimation {
                            to: bar.height * 0.25
                            duration: bar.modelData
                            easing.type: Easing.InQuad
                        }
                    }
                }
            }
        }
    }
}
