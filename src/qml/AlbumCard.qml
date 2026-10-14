// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.MediaCard {
    id: root

    required property string key
    required property string title
    required property string artist
    required property url artworkUrl

    artworkHeight: artworkWidth
    text: title
    subtitle: artist
    source: artworkUrl

    onClicked: T.ApplicationWindow.window.openAlbum(key)

    Kirigami.Icon {
        parent: root.artworkItem
        anchors.centerIn: parent
        width: Math.round(parent.width / 3)
        height: width
        visible: root.artworkItem.status !== Image.Ready
        source: "media-album-cover"
        opacity: 0.4
    }
}
