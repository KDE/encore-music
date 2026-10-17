// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

// The current song at the bottom of the sidebar
T.AbstractButton {
    id: root

    required property BigscreenUI.Sidebar sidebar

    readonly property real artworkSize: Kirigami.Units.iconSizes.large
    readonly property real inset: Math.round((sidebar.railWidth - sidebar.padding * 2 - artworkSize) / 2)

    visible: Player.hasTrack

    implicitHeight: artworkSize + inset * 2
    padding: inset

    focusPolicy: Qt.TabFocus
    hoverEnabled: true

    Accessible.name: i18nc("@action:button %1 is a song title, %2 its artist", "Now playing: %1 by %2", Player.currentTrack.title, Player.currentTrack.artist)

    Keys.onReturnPressed: click()
    Keys.onEnterPressed: click()

    background: BigscreenUI.DelegateBackground {
        control: root
        raisedBackground: false
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Item {
            Layout.preferredWidth: root.artworkSize
            Layout.preferredHeight: root.artworkSize

            // Fades in each song's artwork
            Repeater {
                model: [Player.currentTrack.artworkUrl]

                delegate: Kirigami.ShadowedImage {
                    required property url modelData

                    anchors.fill: parent
                    source: modelData
                    sourceSize.width: width * Screen.devicePixelRatio
                    sourceSize.height: height * Screen.devicePixelRatio
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    radius: Kirigami.Units.cornerRadius
                    color: Kirigami.Theme.alternateBackgroundColor
                    shadow.size: Kirigami.Units.largeSpacing
                    shadow.color: Qt.rgba(0, 0, 0, 0.25)
                    shadow.yOffset: 2

                    opacity: 0
                    Component.onCompleted: opacity = 1
                    Behavior on opacity {
                        OpacityAnimator { duration: Kirigami.Units.longDuration }
                    }

                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: Kirigami.Units.iconSizes.smallMedium
                        height: width
                        visible: parent.status !== Image.Ready
                        source: "media-album-cover"
                        opacity: 0.5
                    }
                }
            }

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: -Kirigami.Units.smallSpacing
                width: indicator.implicitWidth + Kirigami.Units.smallSpacing * 3
                height: width
                radius: width / 2
                color: Kirigami.Theme.backgroundColor
                scale: Player.playing ? 1 : 0
                Behavior on scale {
                    NumberAnimation {
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutBack
                    }
                }

                PlayingIndicator {
                    id: indicator
                    anchors.centerIn: parent
                    implicitHeight: Kirigami.Units.iconSizes.small * 0.75
                    height: implicitHeight
                    playing: Player.playing
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0
            opacity: root.sidebar.expanded ? 1 : 0
            Behavior on opacity {
                OpacityAnimator { duration: Kirigami.Units.shortDuration }
            }

            BigscreenUI.Label {
                Layout.fillWidth: true
                text: Player.currentTrack.title
                elide: Text.ElideRight
                Accessible.ignored: true
            }
            BigscreenUI.Label {
                Layout.fillWidth: true
                visible: text.length > 0
                text: Player.currentTrack.artist
                type: BigscreenUI.Label.Small
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
                Accessible.ignored: true
            }
            BigscreenUI.MediaProgressBar {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                implicitHeight: 3
                value: Player.duration > 0 ? Player.position / Player.duration : 0
                background: Rectangle {
                    radius: height / 2
                    color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.15)
                }
            }
        }
    }
}
