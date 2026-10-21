// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.Page {
    id: root

    readonly property var track: Player.currentTrack

    // Smaller type and controls for handhelds and small windows
    readonly property bool compact: width < Kirigami.Units.gridUnit * 60 || height < Kirigami.Units.gridUnit * 48

    // Too short for the queue as well
    readonly property bool showUpNext: height >= Kirigami.Units.gridUnit * 30

    BigscreenUI.BackHandler.enabled: visible
    BigscreenUI.BackHandler.onActivated: T.StackView.view.pop()

    background: Backdrop {
        source: root.track.artworkUrl
    }

    BigscreenUI.Label {
        anchors.centerIn: parent
        visible: !Player.hasTrack
        text: i18n("Nothing is playing")
        type: BigscreenUI.Label.Section
    }

    RowLayout {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: root.showUpNext ? upNext.top : parent.bottom
        anchors.leftMargin: root.contentMargins.left
        anchors.rightMargin: root.contentMargins.right
        anchors.topMargin: root.contentMargins.top
        anchors.bottomMargin: root.showUpNext ? Kirigami.Units.gridUnit * (root.compact ? 1 : 2) : root.contentMargins.bottom

        visible: Player.hasTrack
        spacing: Kirigami.Units.gridUnit * (root.compact ? 2 : 4)

        Kirigami.ShadowedImage {
            id: artwork

            readonly property real size: Math.min(content.height, content.width * (root.compact ? 0.35 : 0.4))

            Layout.preferredWidth: size
            Layout.preferredHeight: size
            Layout.alignment: Qt.AlignVCenter

            source: root.track.artworkUrl
            sourceSize.width: size * Screen.devicePixelRatio
            sourceSize.height: size * Screen.devicePixelRatio
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            radius: BigscreenUI.Units.cardCornerRadius
            color: Kirigami.Theme.alternateBackgroundColor
            shadow.size: Kirigami.Units.gridUnit * 2
            shadow.yOffset: Kirigami.Units.largeSpacing
            shadow.color: Qt.rgba(0, 0, 0, 0.5)

            Kirigami.Icon {
                anchors.centerIn: parent
                width: Math.min(Kirigami.Units.iconSizes.enormous, artwork.size / 2)
                height: width
                visible: artwork.status !== Image.Ready
                source: "media-album-cover"
                opacity: 0.5
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: Kirigami.Units.smallSpacing

            BigscreenUI.Label {
                Layout.fillWidth: true
                visible: text.length > 0
                text: root.track.year > 0 ? i18nc("@info album title and release year", "%1 · %2", root.track.album, root.track.year) : root.track.album
                type: root.compact ? BigscreenUI.Label.Body : BigscreenUI.Label.Heading
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
            }

            BigscreenUI.Label {
                Layout.fillWidth: true
                text: root.track.title
                type: root.compact ? BigscreenUI.Label.Section : BigscreenUI.Label.Display
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
                Accessible.role: Accessible.Heading
            }

            BigscreenUI.Label {
                Layout.fillWidth: true
                visible: text.length > 0
                text: root.track.artist
                type: root.compact ? BigscreenUI.Label.Heading : BigscreenUI.Label.Section
                elide: Text.ElideRight
            }

            BigscreenUI.SeekBar {
                id: seekBar

                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.gridUnit * (root.compact ? 1 : 2)

                from: 0
                to: Math.max(1, Player.duration)
                stepSize: 10000

                Accessible.name: i18n("Playback position")
                KeyNavigation.down: playButton
                onMoved: Player.seek(value)

                // Follow playback, except while the user drags the handle
                Binding on value {
                    value: Player.position
                    when: !seekBar.pressed
                    restoreMode: Binding.RestoreNone
                }
            }

            RowLayout {
                Layout.fillWidth: true

                BigscreenUI.Label {
                    text: Player.formatTime(Player.position)
                    color: Kirigami.Theme.disabledTextColor
                    font.features: ({ "tnum": 1 })
                }
                Item {
                    Layout.fillWidth: true
                }
                BigscreenUI.Label {
                    text: Player.formatTime(Player.duration)
                    color: Kirigami.Theme.disabledTextColor
                    font.features: ({ "tnum": 1 })
                }
            }

            RowLayout {
                Layout.topMargin: root.compact ? 0 : Kirigami.Units.gridUnit
                spacing: root.compact ? Kirigami.Units.largeSpacing : Kirigami.Units.gridUnit

                BigscreenUI.RoundButton {
                    id: shuffleButton
                    Layout.alignment: Qt.AlignVCenter
                    icon.name: "media-playlist-shuffle"
                    text: i18n("Shuffle")
                    checked: Player.shuffle
                    KeyNavigation.right: previousButton
                    KeyNavigation.up: seekBar
                    KeyNavigation.down: upNext
                    onClicked: Player.shuffle = !Player.shuffle
                }
                BigscreenUI.RoundButton {
                    id: previousButton
                    Layout.alignment: Qt.AlignVCenter
                    icon.name: "media-skip-backward"
                    text: i18n("Previous")
                    enabled: Player.canGoPrevious
                    KeyNavigation.right: playButton
                    KeyNavigation.up: seekBar
                    KeyNavigation.down: upNext
                    onClicked: Player.previous()
                }
                BigscreenUI.RoundButton {
                    id: playButton
                    Layout.alignment: Qt.AlignVCenter
                    focus: true
                    padding: Kirigami.Units.gridUnit * (root.compact ? 1 : 1.5)
                    icon.name: Player.playing ? "media-playback-pause" : "media-playback-start"
                    icon.width: root.compact ? Kirigami.Units.iconSizes.medium : Kirigami.Units.iconSizes.large
                    icon.height: root.compact ? Kirigami.Units.iconSizes.medium : Kirigami.Units.iconSizes.large
                    text: Player.playing ? i18n("Pause") : i18n("Play")
                    KeyNavigation.right: nextButton
                    KeyNavigation.up: seekBar
                    KeyNavigation.down: upNext
                    onClicked: Player.togglePlayPause()
                }
                BigscreenUI.RoundButton {
                    id: nextButton
                    Layout.alignment: Qt.AlignVCenter
                    icon.name: "media-skip-forward"
                    text: i18n("Next")
                    enabled: Player.canGoNext
                    KeyNavigation.right: repeatButton
                    KeyNavigation.up: seekBar
                    KeyNavigation.down: upNext
                    onClicked: Player.next()
                }
                BigscreenUI.RoundButton {
                    id: repeatButton
                    Layout.alignment: Qt.AlignVCenter
                    icon.name: Player.repeatMode === Player.RepeatOne ? "media-playlist-repeat-song" : "media-playlist-repeat"
                    text: {
                        switch (Player.repeatMode) {
                        case Player.RepeatAll: return i18n("Repeat all");
                        case Player.RepeatOne: return i18n("Repeat one");
                        default: return i18n("Repeat off");
                        }
                    }
                    checked: Player.repeatMode !== Player.RepeatNone
                    KeyNavigation.up: seekBar
                    KeyNavigation.down: upNext
                    onClicked: {
                        switch (Player.repeatMode) {
                        case Player.RepeatNone:
                            Player.repeatMode = Player.RepeatAll;
                            break;
                        case Player.RepeatAll:
                            Player.repeatMode = Player.RepeatOne;
                            break;
                        default:
                            Player.repeatMode = Player.RepeatNone;
                            break;
                        }
                    }
                }
            }
        }
    }

    BigscreenUI.ContentRail {
        id: upNext

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.contentMargins.bottom

        visible: Player.hasTrack && root.showUpNext
        title: i18n("Up next")
        model: Player.queue

        KeyNavigation.up: playButton

        delegate: BigscreenUI.MediaCard {
            required property int index
            required property string title
            required property string artist
            required property url artworkUrl
            required property bool isCurrent

            artworkWidth: Kirigami.Units.gridUnit * (root.compact ? 6 : 8)
            artworkHeight: artworkWidth
            text: title
            subtitle: isCurrent ? i18n("Now playing") : artist
            source: artworkUrl

            onClicked: Player.playIndex(index)
        }

        // Show the current track first, unless the user is browsing the queue
        function showCurrent(): void {
            if (!activeFocus) {
                currentIndex = Player.queue.currentIndex;
            }
        }

        Component.onCompleted: showCurrent()

        Connections {
            target: Player.queue

            function onCurrentIndexChanged(): void {
                upNext.showCurrent();
            }
            function onModelReset(): void {
                upNext.showCurrent();
            }
        }
    }
}
