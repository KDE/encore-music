// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.ButtonDelegate {
    id: root

    required property int index
    required property url url
    required property string title
    required property string artist
    required property string album
    required property url artworkUrl
    required property var duration
    required property int trackNumber

    // Show the track number rather than the artwork, as in an album's track list
    property bool showTrackNumber: false

    readonly property bool current: Player.hasTrack && Player.currentTrack.url.toString() === url.toString()

    width: ListView.view ? ListView.view.rowWidth : implicitWidth
    text: title

    leading: Item {
        implicitWidth: root.showTrackNumber ? Kirigami.Units.gridUnit * 1.5 : Kirigami.Units.iconSizes.large
        implicitHeight: root.showTrackNumber ? numberLabel.implicitHeight : Kirigami.Units.iconSizes.large

        BigscreenUI.Label {
            id: numberLabel
            anchors.fill: parent
            visible: root.showTrackNumber && !root.current
            text: root.trackNumber > 0 ? root.trackNumber : ""
            horizontalAlignment: Text.AlignRight
            color: Kirigami.Theme.disabledTextColor
            font.features: ({ "tnum": 1 })
        }

        Kirigami.ShadowedImage {
            id: artwork
            anchors.fill: parent
            visible: !root.showTrackNumber
            source: visible ? root.artworkUrl : ""
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            radius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.alternateBackgroundColor

            Kirigami.Icon {
                anchors.centerIn: parent
                width: Kirigami.Units.iconSizes.small
                height: width
                visible: artwork.status !== Image.Ready && !root.current
                source: "view-media-track"
                opacity: 0.5
            }

            Rectangle {
                anchors.fill: parent
                visible: root.current
                radius: parent.radius
                color: Qt.rgba(0, 0, 0, 0.5)
            }
        }

        PlayingIndicator {
            x: root.showTrackNumber ? parent.width - width : Math.round((parent.width - width) / 2)
            anchors.verticalCenter: parent.verticalCenter
            visible: root.current
            playing: Player.playing
            color: root.showTrackNumber ? Kirigami.Theme.highlightColor : "white"
        }
    }

    trailing: BigscreenUI.Label {
        text: root.duration > 0 ? Player.formatTime(root.duration) : ""
        color: Kirigami.Theme.disabledTextColor
        font.features: ({ "tnum": 1 })
    }
}
