// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.Page {
    id: root

    readonly property bool empty: Library.trackCount === 0
    readonly property url backdropSource: rails.currentItem?.currentItem?.artworkUrl ?? ""

    property string searchText

    LibraryPlaceholder {
        anchors.fill: parent
        visible: root.empty
        focus: root.empty
        title: i18n("Search")
        iconName: "search"
    }

    ArtistModel {
        id: artists
        searchText: root.searchText
    }
    AlbumModel {
        id: albums
        searchText: root.searchText
    }
    TrackModel {
        id: songs
        searchText: root.searchText
    }

    Timer {
        id: searchTimer
        interval: 250
        onTriggered: root.searchText = searchField.text.trim()
    }

    ColumnLayout {
        anchors.fill: parent
        visible: !root.empty
        spacing: 0

        BigscreenUI.TextField {
            id: searchField

            Layout.fillWidth: true
            Layout.leftMargin: root.contentMargins.left
            Layout.rightMargin: root.contentMargins.right
            Layout.topMargin: root.contentMargins.top

            focus: !root.empty
            placeholderText: i18n("Search songs, albums and artists")
            inputMethodHints: Qt.ImhNoPredictiveText
            Accessible.name: i18n("Search")

            onTextChanged: searchTimer.restart()
            onAccepted: {
                searchTimer.stop();
                root.searchText = text.trim();
                if (rails.count > 0) {
                    rails.forceActiveFocus();
                }
            }
            KeyNavigation.down: rails.count > 0 ? rails : null
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            BigscreenUI.PlaceholderMessage {
                anchors.centerIn: parent
                width: Math.min(parent.width - root.contentMargins.left - root.contentMargins.right, implicitWidth)
                visible: rails.count === 0
                iconName: "search"
                text: root.searchText.length > 0 ? i18n("No results for “%1”", root.searchText) : i18n("Search your music")
                explanation: root.searchText.length > 0 ? i18n("Try a different spelling, or fewer words.") : i18n("Find songs, albums and artists.")
            }

            BigscreenUI.RailView {
                id: rails

                anchors.fill: parent
                bottomMargin: root.contentMargins.bottom
                visible: count > 0

                Keys.priority: Keys.AfterItem
                Keys.onUpPressed: searchField.forceActiveFocus()

                BigscreenUI.ContentRail {
                    title: i18n("Artists")
                    model: root.searchText.length > 0 ? artists : null
                    delegate: ArtistCard {
                        artworkWidth: Kirigami.Units.gridUnit * 8
                    }
                }

                BigscreenUI.ContentRail {
                    title: i18n("Albums")
                    model: root.searchText.length > 0 ? albums : null
                    delegate: AlbumCard {
                        artworkWidth: Kirigami.Units.gridUnit * 10
                    }
                }

                BigscreenUI.ContentRail {
                    title: i18n("Songs")
                    model: root.searchText.length > 0 ? songs : null
                    delegate: BigscreenUI.MediaCard {
                        id: songCard

                        required property int index
                        required property string title
                        required property string artist
                        required property url artworkUrl

                        artworkWidth: Kirigami.Units.gridUnit * 10
                        artworkHeight: artworkWidth
                        text: title
                        subtitle: artist
                        source: artworkUrl

                        onClicked: songs.play(index)

                        Kirigami.Icon {
                            parent: songCard.artworkItem
                            anchors.centerIn: parent
                            width: Math.round(parent.width / 3)
                            height: width
                            visible: songCard.artworkItem.status !== Image.Ready
                            source: "view-media-track"
                            opacity: 0.4
                        }
                    }
                }
            }
        }
    }
}
