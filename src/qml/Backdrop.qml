// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Effects

import org.kde.kirigami as Kirigami
import org.kde.bigscreen.ui as BigscreenUI

Item {
    id: root

    property url source

    // How long image stays before changing so it doesn't flash around everywhere
    property int delay: 0

    // The source being shown, once the delay has passed
    property url shownSource
    property ArtworkLayer front: layerA
    property ArtworkLayer back: layerB
    readonly property bool showingArtwork: layerA.opacity > 0 || layerB.opacity > 0

    Component.onCompleted: shownSource = source
    onSourceChanged: {
        if (delay > 0) {
            delayTimer.restart();
        } else {
            shownSource = source;
        }
    }
    onShownSourceChanged: {
        if (front.source.toString() === shownSource.toString() && front.opacity > 0) {
            back.opacity = 0;
            return;
        }
        if (shownSource.toString() === "") {
            front.opacity = 0;
            back.opacity = 0;
            return;
        }
        back.source = shownSource;
        if (back.ready) {
            swap();
        }
    }

    function swap(): void {
        const previous = front;
        front = back;
        back = previous;
        front.opacity = 1;
        back.opacity = 0;
    }

    Timer {
        id: delayTimer
        interval: root.delay
        onTriggered: root.shownSource = root.source
    }

    // Shown when there is no artwork
    BigscreenUI.AmbientBackground {
        anchors.fill: parent
        intensity: root.showingArtwork ? 0 : 0.45
        Behavior on intensity {
            NumberAnimation {
                duration: Kirigami.Units.veryLongDuration
            }
        }
    }

    ArtworkLayer {
        id: layerA
        anchors.fill: parent
        onReadyChanged: if (ready && root.back === layerA && layerA.source.toString() === root.shownSource.toString()) root.swap()
    }

    ArtworkLayer {
        id: layerB
        anchors.fill: parent
        onReadyChanged: if (ready && root.back === layerB && layerB.source.toString() === root.shownSource.toString()) root.swap()
    }

    // Keeps text over the artwork readable
    Rectangle {
        anchors.fill: parent
        opacity: root.showingArtwork ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.veryLongDuration
            }
        }
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.2)
            }
            GradientStop {
                position: 1
                color: Qt.alpha(Kirigami.Theme.backgroundColor, 0.85)
            }
        }
    }

    // Artwork blurred
    component ArtworkLayer: Item {
        id: layer

        property alias source: image.source
        readonly property bool ready: image.status === Image.Ready

        opacity: 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.veryLongDuration
            }
        }

        Image {
            id: image
            anchors.fill: parent
            // Blurred anyway, so a small image is enough and cheap to blur
            sourceSize.width: 256
            sourceSize.height: 256
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            visible: false
        }

        MultiEffect {
            anchors.fill: parent
            source: image
            opacity: 0.5
            autoPaddingEnabled: false
            blurEnabled: true
            blur: 1.0
            blurMax: 64
            saturation: 0.2
        }
    }
}
