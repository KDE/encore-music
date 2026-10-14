// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.ScrollablePage {
    id: root

    title: i18n("Settings")

    BigscreenUI.BackHandler.enabled: visible
    BigscreenUI.BackHandler.onActivated: T.StackView.view.pop()

    readonly property string homePath: StandardPaths.writableLocation(StandardPaths.HomeLocation).toString().replace("file://", "")

    // Shortens paths in the home folder, such as ~/Music
    function displayPath(path: string): string {
        return path.startsWith(homePath + "/") ? "~" + path.slice(homePath.length) : path;
    }

    ColumnLayout {
        id: column

        width: root.availableWidth
        spacing: Kirigami.Units.smallSpacing

        BigscreenUI.KeyNavigationGroup.orientation: Qt.Vertical

        BigscreenUI.Label {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            text: i18n("Music folders")
            type: BigscreenUI.Label.Heading
            color: Kirigami.Theme.disabledTextColor
        }

        Repeater {
            model: Library.folders

            delegate: BigscreenUI.ButtonDelegate {
                required property string modelData

                Layout.fillWidth: true
                text: root.displayPath(modelData)
                icon.name: "folder-music"
                trailing: Kirigami.Icon {
                    source: "list-remove"
                    implicitWidth: Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small
                }
                Accessible.description: i18n("Remove from the library")
                onClicked: {
                    removeDialog.folder = modelData;
                    removeDialog.open();
                }
            }
        }

        BigscreenUI.ButtonDelegate {
            id: addFolderButton
            Layout.fillWidth: true
            focus: true
            text: i18n("Add folder…")
            icon.name: "list-add"
            onClicked: folderDialog.open()
        }

        BigscreenUI.ButtonDelegate {
            Layout.fillWidth: true
            text: Library.scanning ? i18n("Looking for music…") : i18n("Check for new music")
            description: Library.scanning ? i18np("Checked 1 file", "Checked %1 files", Library.scannedFiles)
                                          : i18np("1 song in your library", "%1 songs in your library", Library.trackCount)
            icon.name: "view-refresh"
            enabled: !Library.scanning
            onClicked: Library.rescan()
        }

        BigscreenUI.Label {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.gridUnit * 2
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            text: i18n("About")
            type: BigscreenUI.Label.Heading
            color: Kirigami.Theme.disabledTextColor
        }

        BigscreenUI.TextDelegate {
            Layout.fillWidth: true
            text: i18n("Music")
            description: i18n("Version %1", Qt.application.version)
        }
    }

    Loader {
        id: folderDialog
        active: false
        sourceComponent: FolderDialog {
            title: i18n("Add Music Folder")
            currentFolder: StandardPaths.writableLocation(StandardPaths.MusicLocation)
            onAccepted: Library.addFolder(selectedFolder)
        }

        function open(): void {
            active = true;
            item.open();
        }
    }

    BigscreenUI.Dialog {
        id: removeDialog

        property string folder

        title: i18n("Remove %1?", root.displayPath(folder))
        standardButtons: T.Dialog.Cancel | T.Dialog.Ok

        contentItem: BigscreenUI.Label {
            text: i18n("Its music will no longer appear in your library. The files themselves are not deleted.")
            wrapMode: Text.Wrap
        }

        openFocusItem: standardButton(T.Dialog.Cancel)
        Component.onCompleted: standardButton(T.Dialog.Ok).text = i18n("Remove")

        onAccepted: Library.removeFolder(folder)
        onClosed: addFolderButton.forceActiveFocus()
    }
}
