// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

// An album's cover, details and songs
BigscreenUI.Page {
    id: root

    required property string albumKey

    readonly property var album: tracks.album

    BigscreenUI.BackHandler.enabled: visible
    BigscreenUI.BackHandler.onActivated: T.StackView.view.pop()

    function formatLength(milliseconds: real): string {
        const minutes = Math.round(milliseconds / 60000);
        if (minutes < 60) {
            return i18ncp("@info length of an album", "1 min", "%1 min", minutes);
        }
        return i18nc("@info length of an album, in hours and minutes", "%1 hr %2 min", Math.floor(minutes / 60), minutes % 60);
    }

    TrackModel {
        id: tracks
        albumKey: root.albumKey
    }

    background: Backdrop {
        source: root.album.artworkUrl
    }

    Kirigami.ShadowedImage {
        id: cover

        x: root.contentMargins.left
        y: root.contentMargins.top + Kirigami.Units.gridUnit * 2
        width: Math.min(Kirigami.Units.gridUnit * 26, root.width * 0.3)
        height: width

        source: root.album.artworkUrl
        sourceSize.width: width * Screen.devicePixelRatio
        sourceSize.height: height * Screen.devicePixelRatio
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        radius: BigscreenUI.Units.cardCornerRadius
        color: Kirigami.Theme.alternateBackgroundColor
        shadow.size: Kirigami.Units.gridUnit * 2
        shadow.yOffset: Kirigami.Units.largeSpacing
        shadow.color: Qt.rgba(0, 0, 0, 0.5)

        Kirigami.Icon {
            anchors.centerIn: parent
            width: Kirigami.Units.iconSizes.enormous
            height: width
            visible: cover.status !== Image.Ready
            source: "media-album-cover"
            opacity: 0.4
        }
    }

    BigscreenUI.HeroBanner {
        id: hero

        anchors.left: cover.right
        anchors.leftMargin: Kirigami.Units.gridUnit * 3
        anchors.right: parent.right
        anchors.rightMargin: root.contentMargins.right
        anchors.top: cover.top

        overline: i18n("Album")
        title: root.album.title
        titlePixelSize: BigscreenUI.Units.titleFontPixelSize
        metadata: [
            root.album.artist,
            root.album.year > 0 ? String(root.album.year) : "",
            i18np("1 song", "%1 songs", tracks.count),
            root.album.duration > 0 ? root.formatLength(root.album.duration) : "",
        ]

        BigscreenUI.Button {
            id: playButton
            focus: true
            text: i18n("Play")
            icon.name: "media-playback-start"
            KeyNavigation.down: trackList
            onClicked: tracks.play(0)
        }
        BigscreenUI.Button {
            id: shuffleButton
            text: i18n("Shuffle")
            icon.name: "media-playlist-shuffle"
            KeyNavigation.down: trackList
            onClicked: tracks.shuffle()
        }
        BigscreenUI.Button {
            id: artistButton
            text: i18n("Go to artist")
            icon.name: "view-media-artist"
            KeyNavigation.down: trackList
            onClicked: root.T.ApplicationWindow.window.openArtist(root.album.artistKey)
        }
    }

    BigscreenUI.ListView {
        id: trackList

        anchors.top: hero.bottom
        anchors.topMargin: Kirigami.Units.gridUnit
        anchors.left: hero.left
        anchors.leftMargin: -Kirigami.Units.gridUnit
        anchors.right: hero.right
        anchors.rightMargin: -Kirigami.Units.gridUnit
        anchors.bottom: parent.bottom

        leftMargin: Kirigami.Units.gridUnit
        rightMargin: Kirigami.Units.gridUnit
        bottomMargin: root.contentMargins.bottom + Kirigami.Units.gridUnit

        Keys.priority: Keys.AfterItem
        Keys.onUpPressed: playButton.forceActiveFocus()

        section.property: root.album.discCount > 1 ? "discNumber" : ""
        section.delegate: BigscreenUI.Label {
            required property string section

            leftPadding: Kirigami.Units.gridUnit
            topPadding: Kirigami.Units.gridUnit
            bottomPadding: Kirigami.Units.smallSpacing
            text: i18n("Disc %1", section)
            type: BigscreenUI.Label.Heading
            color: Kirigami.Theme.disabledTextColor
        }

        model: tracks
        delegate: TrackDelegate {
            showTrackNumber: true
            // Songs on a compilation have their own artists
            description: artist !== root.album.artist ? artist : ""
            onClicked: tracks.play(index)
        }
    }
}
