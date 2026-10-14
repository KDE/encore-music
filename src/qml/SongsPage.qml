// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.Page {
    id: root

    readonly property bool empty: Library.trackCount === 0
    readonly property url backdropSource: empty ? "" : list.currentItem?.artworkUrl ?? ""

    LibraryPlaceholder {
        anchors.fill: parent
        visible: root.empty
        focus: root.empty
        title: i18n("Songs")
        iconName: "view-media-track"
    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout {
            id: header
            Layout.fillWidth: true
            Layout.leftMargin: root.contentMargins.left
            Layout.rightMargin: root.contentMargins.right
            Layout.topMargin: root.contentMargins.top

            visible: !root.empty
            spacing: Kirigami.Units.gridUnit

            PageHeading {
                title: i18n("Songs")
                summary: i18np("1 song", "%1 songs", songs.count)
            }
            Item {
                Layout.fillWidth: true
            }

            BigscreenUI.Button {
                id: shuffleButton
                focus: !root.empty
                text: i18n("Shuffle all")
                icon.name: "media-playlist-shuffle"
                KeyNavigation.down: list
                onClicked: songs.shuffle()
            }
        }

        BigscreenUI.ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: !root.empty

            reuseItems: true

            Keys.priority: Keys.AfterItem
            Keys.onUpPressed: shuffleButton.forceActiveFocus()

            model: TrackModel {
                id: songs
            }
            delegate: TrackDelegate {
                description: album.length > 0 ? i18nc("@info song artist and album", "%1 · %2", artist, album) : artist
                onClicked: songs.play(index)
            }
        }
    }
}
