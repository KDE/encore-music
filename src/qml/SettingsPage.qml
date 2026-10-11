// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

BigscreenUI.ScrollablePage {
    id: root

    title: i18n("Settings")

    BigscreenUI.BackHandler.enabled: visible
    BigscreenUI.BackHandler.onActivated: T.StackView.view.pop()

    ColumnLayout {
        width: root.availableWidth
        spacing: Kirigami.Units.smallSpacing

        BigscreenUI.Label {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            text: i18n("About")
            type: BigscreenUI.Label.Heading
            color: Kirigami.Theme.disabledTextColor
        }

        BigscreenUI.TextDelegate {
            Layout.fillWidth: true
            focus: true
            text: i18n("Music")
            description: i18n("Version %1", Qt.application.version)
        }
    }
}
