// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.ApplicationWindow {
    id: root

    width: 1280
    height: 700
    visible: true
    title: i18n("Encore Music")

    // Whether we are in "narrow" mode (for handhelds, smaller screens)
    readonly property bool narrow: width < Kirigami.Units.gridUnit * 75

    component Section: Kirigami.Action {
        required property Item page
    }

    readonly property list<Section> sections: [
        Section {
            text: i18n("Home")
            icon.name: "go-home-symbolic"
            page: HomePage {}
        },
        Section {
            text: i18n("Albums")
            icon.name: "media-album-cover"
            page: AlbumsPage {}
        },
        Section {
            text: i18n("Artists")
            icon.name: "view-media-artist"
            page: ArtistsPage {}
        },
        Section {
            text: i18n("Songs")
            icon.name: "view-media-track"
            page: SongsPage {}
        },
        Section {
            text: i18n("Playlists")
            icon.name: "view-media-playlist"
            page: PlaceholderPage {
                title: i18n("Playlists")
                iconName: "view-media-playlist"
                text: i18n("No playlists yet")
                explanation: i18n("Playlists you create will appear here.")
            }
        },
        Section {
            text: i18n("Search")
            icon.name: "search"
            displayHint: Kirigami.DisplayHint.IconOnly
            page: SearchPage {}
        }
    ]

    property int currentIndex: 0
    readonly property Item currentPage: sections[currentIndex].page
    readonly property Item navigation: narrow ? sectionSidebar : tabBar

    function showSection(index: int): void {
        currentIndex = index;
        tabBar.currentIndex = index;
        sectionSidebar.currentIndex = index;
        showPage(currentPage);
    }

    function openNowPlaying(): void {
        if (!(pageStack.currentItem instanceof NowPlayingPage)) {
            pageStack.push(nowPlayingPage);
        }
    }

    function openAlbum(key: string): void {
        pageStack.push(albumPage, { albumKey: key });
    }

    function openArtist(key: string): void {
        // Going to the artist of an album opened from that artist goes back to them
        const previous = pageStack.depth > 1 ? pageStack.get(pageStack.depth - 2) : null;
        if (previous instanceof ArtistPage && previous.artistKey === key) {
            pageStack.pop();
        } else {
            pageStack.push(artistPage, { artistKey: key });
        }
    }

    function openSettings(): void {
        if (!(pageStack.currentItem instanceof SettingsPage)) {
            pageStack.push(settingsPage);
        }
    }

    initialPage: currentPage

    navigationBar: narrow ? null : tabBar
    sidebar: narrow ? sectionSidebar : null

    // Keep focus on the navigation when switching layouts
    onNarrowChanged: {
        if (tabBar.activeFocus || sectionSidebar.activeFocus) {
            navigation.forceActiveFocus();
        }
    }

    BigscreenUI.BackHandler.enabled: pageStack.depth <= 1
    BigscreenUI.BackHandler.onActivated: {
        if (!navigation.activeFocus) {
            navigation.forceActiveFocus();
        } else if (currentIndex !== 0) {
            showSection(0);
        } else {
            root.quitDialog.open()
        }
    }

    // The artwork of whatever is selected in the current section
    Backdrop {
        anchors.fill: parent
        z: -1
        delay: 200
        source: root.currentPage.backdropSource ?? ""
    }

    readonly property list<Item> navigationItems: [
        BigscreenUI.NavigationBar {
            id: tabBar

            leading: RowLayout {
                spacing: Kirigami.Units.largeSpacing * 2

                Kirigami.Icon {
                    implicitWidth: Kirigami.Units.iconSizes.medium
                    implicitHeight: Kirigami.Units.iconSizes.medium
                    source: "org.kde.encoremusic"
                }
                BigscreenUI.Label {
                    text: root.title
                    type: BigscreenUI.Label.Heading
                    font.weight: Font.Light
                }
            }

            actions: root.sections
            onActivated: index => root.showSection(index)

            trailing: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                NowPlayingButton {
                    onClicked: root.openNowPlaying()
                }

                BigscreenUI.NavigationTab {
                    action: Kirigami.Action {
                        text: i18n("Settings")
                        icon.name: "configure"
                        displayHint: Kirigami.DisplayHint.IconOnly
                        onTriggered: root.openSettings()
                    }
                }
            }
        },

        BigscreenUI.Sidebar {
            id: sectionSidebar

            title: root.title
            iconName: "org.kde.encoremusic"

            actions: [...root.sections, separatorAction, settingsAction]

            footer: NowPlayingEntry {
                sidebar: sectionSidebar
                onClicked: root.openNowPlaying()
            }
            onActivated: index => {
                if (index < root.sections.length) {
                    root.showSection(index);
                } else {
                    currentIndex = root.currentIndex;
                }
            }

            Kirigami.Action {
                id: separatorAction
                separator: true
            }
            Kirigami.Action {
                id: settingsAction
                text: i18n("Settings")
                icon.name: "configure"
                onTriggered: root.openSettings()
            }
        }
    ]

    BigscreenUI.Toast {
        id: toast
        z: 1
        timeout: 4000
        anchors.bottomMargin: BigscreenUI.Units.verticalScreenMargin
    }

    Connections {
        target: Player

        function onErrorOccurred(message: string): void {
            toast.show(message, "dialog-warning");
        }
    }

    Component {
        id: nowPlayingPage
        NowPlayingPage {}
    }

    Component {
        id: albumPage
        AlbumPage {}
    }

    Component {
        id: artistPage
        ArtistPage {}
    }

    Component {
        id: settingsPage
        SettingsPage {}
    }

    Shortcut {
        sequence: "Toggle Media Play/Pause"
        context: Qt.ApplicationShortcut
        onActivated: Player.togglePlayPause()
    }
    Shortcut {
        sequence: "Media Play"
        context: Qt.ApplicationShortcut
        onActivated: Player.play()
    }
    Shortcut {
        sequence: "Media Pause"
        context: Qt.ApplicationShortcut
        onActivated: Player.pause()
    }
    Shortcut {
        sequence: "Media Stop"
        context: Qt.ApplicationShortcut
        onActivated: Player.stop()
    }
    Shortcut {
        sequence: "Media Next"
        context: Qt.ApplicationShortcut
        onActivated: Player.next()
    }
    Shortcut {
        sequence: "Media Previous"
        context: Qt.ApplicationShortcut
        onActivated: Player.previous()
    }
}
