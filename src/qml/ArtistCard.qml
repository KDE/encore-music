// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.MediaCard {
    id: root

    required property string key
    required property string name
    required property url artworkUrl
    required property int albumCount

    artworkHeight: artworkWidth
    artworkItem.radius: artworkWidth / 2
    text: name
    subtitle: i18np("1 album", "%1 albums", albumCount)
    source: artworkUrl

    onClicked: T.ApplicationWindow.window.openArtist(key)

    Kirigami.Icon {
        parent: root.artworkItem
        anchors.centerIn: parent
        width: Math.round(parent.width / 3)
        height: width
        visible: root.artworkItem.status !== Image.Ready
        source: "view-media-artist"
        opacity: 0.4
    }
}
