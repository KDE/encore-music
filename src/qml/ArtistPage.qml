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

    required property string artistKey

    readonly property var artist: tracks.artist

    BigscreenUI.BackHandler.enabled: visible
    BigscreenUI.BackHandler.onActivated: T.StackView.view.pop()

    TrackModel {
        id: tracks
        artistKey: root.artistKey
    }

    background: Backdrop {
        source: root.artist.artworkUrl
    }

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        RowLayout {
            id: header
            spacing: 0
            Layout.topMargin: root.contentMargins.top + Kirigami.Units.gridUnit
            Layout.leftMargin: root.contentMargins.left
            Layout.rightMargin: root.contentMargins.right

            Kirigami.ShadowedImage {
                id: portrait
                width: Kirigami.Units.gridUnit * 12
                height: width

                source: root.artist.artworkUrl
                sourceSize.width: width * Screen.devicePixelRatio
                sourceSize.height: height * Screen.devicePixelRatio
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                radius: width / 2
                color: Kirigami.Theme.alternateBackgroundColor
                shadow.size: Kirigami.Units.gridUnit * 2
                shadow.yOffset: Kirigami.Units.largeSpacing
                shadow.color: Qt.rgba(0, 0, 0, 0.5)

                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: Kirigami.Units.iconSizes.huge
                    height: width
                    visible: portrait.status !== Image.Ready
                    source: "view-media-artist"
                    opacity: 0.4
                }
            }

            BigscreenUI.HeroBanner {
                id: hero

                Layout.leftMargin: Kirigami.Units.gridUnit * 3
                Layout.alignment: Qt.AlignVCenter

                overline: i18n("Artist")
                title: root.artist.name
                titlePixelSize: BigscreenUI.Units.titleFontPixelSize
                metadata: [
                    i18np("1 album", "%1 albums", root.artist.albumCount),
                    i18np("1 song", "%1 songs", root.artist.trackCount),
                ]

                BigscreenUI.Button {
                    id: playButton
                    focus: true
                    text: i18n("Play")
                    icon.name: "media-playback-start"
                    KeyNavigation.down: grid
                    onClicked: tracks.play(0)
                }
                BigscreenUI.Button {
                    id: shuffleButton
                    text: i18n("Shuffle")
                    icon.name: "media-playlist-shuffle"
                    KeyNavigation.down: grid
                    onClicked: tracks.shuffle()
                }
            }
        }

        BigscreenUI.MediaGrid {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.gridUnit * 2

            // Focus starts on the play button
            focus: false

            artworkAspectRatio: 1
            minimumCardWidth: Kirigami.Units.gridUnit * 12

            Keys.priority: Keys.AfterItem
            Keys.onUpPressed: playButton.forceActiveFocus()

            model: AlbumModel {
                artistKey: root.artistKey
            }
            delegate: AlbumCard {
                required property int year

                artworkWidth: grid.cardWidth
                artworkHeight: grid.artworkHeight
                subtitle: year > 0 ? String(year) : ""
            }
        }
    }
}
