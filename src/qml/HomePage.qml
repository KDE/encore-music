// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.Page {
    id: root

    readonly property bool empty: Library.trackCount === 0
    readonly property url backdropSource: empty ? "" : rails.currentItem?.currentItem?.artworkUrl ?? ""

    LibraryPlaceholder {
        anchors.fill: parent
        visible: root.empty
        focus: root.empty
        title: i18n("Home")
        iconName: "go-home"
    }

    BigscreenUI.RailView {
        id: rails

        anchors.fill: parent
        anchors.topMargin: root.contentMargins.top - topMargin
        visible: !root.empty
        focus: !root.empty
        bottomMargin: root.contentMargins.bottom

        FocusScope {
            implicitHeight: hero.implicitHeight

            BigscreenUI.HeroBanner {
                id: hero

                x: root.contentMargins.left
                width: parent.width - x - root.contentMargins.right

                overline: i18n("Your music")
                title: i18n("Welcome")
                titlePixelSize: BigscreenUI.Units.titleFontPixelSize
                metadata: [
                    i18np("1 song", "%1 songs", Library.trackCount),
                    i18np("1 album", "%1 albums", Library.albumCount),
                    i18np("1 artist", "%1 artists", Library.artistCount),
                ]

                BigscreenUI.Button {
                    focus: true
                    text: i18n("Shuffle all")
                    icon.name: "media-playlist-shuffle"
                    onClicked: allSongs.shuffle()
                }
            }
        }

        BigscreenUI.ContentRail {
            title: i18n("Recently added")
            model: AlbumModel {
                sortOrder: AlbumModel.RecentlyAdded
                limit: 20
            }
            delegate: AlbumCard {
                artworkWidth: Kirigami.Units.gridUnit * 12
            }
        }

        BigscreenUI.ContentRail {
            title: i18n("Artists")
            model: ArtistModel {}
            delegate: ArtistCard {
                artworkWidth: Kirigami.Units.gridUnit * 10
            }
        }
    }

    TrackModel {
        id: allSongs
    }
}
