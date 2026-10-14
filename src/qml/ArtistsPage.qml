// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

// Every album artist, by name
BigscreenUI.Page {
    id: root

    readonly property bool empty: Library.trackCount === 0
    readonly property url backdropSource: empty ? "" : grid.currentItem?.artworkUrl ?? ""

    LibraryPlaceholder {
        anchors.fill: parent
        visible: root.empty
        focus: root.empty
        title: i18n("Artists")
        iconName: "view-media-artist"
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
            title: i18n("Artists")
            summary: i18np("1 artist", "%1 artists", grid.count)
        }

        BigscreenUI.MediaGrid {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            visible: !root.empty
            focus: !root.empty

            artworkAspectRatio: 1
            minimumCardWidth: Kirigami.Units.gridUnit * 10

            model: ArtistModel {}
            delegate: ArtistCard {
                artworkWidth: grid.cardWidth
                artworkHeight: grid.artworkHeight
            }
        }
    }
}
