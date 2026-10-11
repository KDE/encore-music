// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

// The top-level sections of the app, switched with the navigation bar
FocusScope {
    id: root

    // Pages leave up unhandled at their top, so it reaches the navigation bar
    Keys.onUpPressed: navigationBar.forceActiveFocus()

    BigscreenUI.BackHandler.enabled: visible
    BigscreenUI.BackHandler.onActivated: {
        if (!navigationBar.activeFocus) {
            navigationBar.forceActiveFocus();
        } else if (navigationBar.currentIndex !== 0) {
            navigationBar.currentIndex = 0;
        }
    }

    StackLayout {
        id: pages

        anchors.fill: parent
        // Pages fill the window under the navigation bar, and keep their content below it
        SafeArea.additionalMargins.top: navigationBar.height
        currentIndex: navigationBar.currentIndex

        PlaceholderPage {
            title: i18n("Home")
            iconName: "go-home"
            text: i18n("Welcome to Music")
            explanation: i18n("Your recently played and recently added music will appear here.")
        }
        PlaceholderPage {
            title: i18n("Albums")
            iconName: "media-album-cover"
            text: i18n("No albums yet")
            explanation: i18n("Albums in your music folders will appear here.")
        }
        PlaceholderPage {
            title: i18n("Artists")
            iconName: "view-media-artist"
            text: i18n("No artists yet")
            explanation: i18n("Artists in your music folders will appear here.")
        }
        PlaceholderPage {
            title: i18n("Songs")
            iconName: "view-media-track"
            text: i18n("No songs yet")
            explanation: i18n("Songs in your music folders will appear here.")
        }
        PlaceholderPage {
            title: i18n("Playlists")
            iconName: "view-media-playlist"
            text: i18n("No playlists yet")
            explanation: i18n("Playlists you create will appear here.")
        }
        PlaceholderPage {
            title: i18n("Search")
            iconName: "search"
            text: i18n("Search your music")
            explanation: i18n("Find songs, albums and artists.")
        }
    }

    BigscreenUI.NavigationBar {
        id: navigationBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        focus: true

        Keys.onDownPressed: pages.children[pages.currentIndex].forceActiveFocus()

        leading: RowLayout {
            spacing: Kirigami.Units.largeSpacing * 2

            Kirigami.Icon {
                implicitWidth: Kirigami.Units.iconSizes.medium
                implicitHeight: Kirigami.Units.iconSizes.medium
                source: "folder-music"
            }
            BigscreenUI.Label {
                text: i18n("Music")
                type: BigscreenUI.Label.Heading
                font.weight: Font.Light
            }
        }

        actions: [
            Kirigami.Action {
                text: i18n("Home")
            },
            Kirigami.Action {
                text: i18n("Albums")
            },
            Kirigami.Action {
                text: i18n("Artists")
            },
            Kirigami.Action {
                text: i18n("Songs")
            },
            Kirigami.Action {
                text: i18n("Playlists")
            },
            Kirigami.Action {
                text: i18n("Search")
                icon.name: "search"
                displayHint: Kirigami.DisplayHint.IconOnly
            }
        ]

        trailing: RowLayout {
            spacing: Kirigami.Units.largeSpacing

            NowPlayingButton {
                id: nowPlayingButton
                KeyNavigation.right: settingsButton
                onClicked: root.T.ApplicationWindow.window.openNowPlaying()
            }

            BigscreenUI.NavigationTab {
                id: settingsButton
                // Left past the now playing button, or when it is hidden, returns to the tabs
                KeyNavigation.left: nowPlayingButton.visible ? nowPlayingButton : null
                action: Kirigami.Action {
                    text: i18n("Settings")
                    icon.name: "configure"
                    displayHint: Kirigami.DisplayHint.IconOnly
                    onTriggered: root.T.ApplicationWindow.window.openSettings()
                }
            }
        }
    }
}
