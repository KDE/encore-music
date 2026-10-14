// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

PlaceholderPage {
    id: root

    text: Library.scanning ? i18n("Looking for music…") : i18n("No music found")
    explanation: {
        if (Library.scanning) {
            return Library.scannedFiles > 0 ? i18np("Checked 1 file", "Checked %1 files", Library.scannedFiles) : "";
        }
        if (Library.folders.length === 0) {
            return i18n("Choose a folder with your music in Settings.");
        }
        return i18np("Add music to %2, or choose another folder in Settings.",
                     "Add music to one of your music folders, or choose another folder in Settings.",
                     Library.folders.length, Library.folders[0]);
    }

    helpfulAction: Kirigami.Action {
        text: i18n("Choose music folders")
        icon.name: "folder-music"
        visible: !Library.scanning
        onTriggered: root.T.ApplicationWindow.window.openSettings()
    }
}
