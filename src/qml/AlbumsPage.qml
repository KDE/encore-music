// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.Page {
    id: root
    readonly property url backdropSource: empty ? "" : grid.currentItem?.artworkUrl ?? ""

    readonly property bool empty: Library.trackCount === 0

    LibraryPlaceholder {
        anchors.fill: parent
        visible: root.empty
        focus: root.empty
        title: i18n("Albums")
        iconName: "media-album-cover"
    }

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        PageHeading {
            id: heading
            Layout.topMargin: root.contentMargins.top
            Layout.leftMargin: root.contentMargins.left
            Layout.rightMargin: root.contentMargins.right
            visible: !root.empty
            title: i18n("Albums")
            summary: i18np("1 album", "%1 albums", grid.count)
        }

        BigscreenUI.MediaGrid {
            id: grid

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: !root.empty
            focus: !root.empty

            artworkAspectRatio: 1
            minimumCardWidth: Kirigami.Units.gridUnit * 12

            model: AlbumModel {}
            delegate: AlbumCard {
                artworkWidth: grid.cardWidth
                artworkHeight: grid.artworkHeight
            }
        }
    }
}
