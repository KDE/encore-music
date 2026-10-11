// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "version.h"

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        qputenv("QT_QUICK_CONTROLS_STYLE", "org.kde.desktop");
    }

    QGuiApplication application(argc, argv);

    KLocalizedString::setApplicationDomain("bigscreen-music");
    KAboutData aboutData(u"bigscreen-music"_s,
                         i18n("Music"),
                         QStringLiteral(BIGSCREENMUSIC_VERSION_STRING),
                         i18n("Music player for Plasma Bigscreen"),
                         KAboutLicense::GPL_V2,
                         i18n("© 2026 KDE Community"));
    aboutData.addAuthor(i18n("Devin Lin"), QString(), u"devin@kde.org"_s);
    aboutData.setLicense(KAboutLicense::GPL_V2, KAboutLicense::OrLaterVersions);
    aboutData.setDesktopFileName(u"org.kde.bigscreen.music"_s);
    KAboutData::setApplicationData(aboutData);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"folder-music"_s));

    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.loadFromModule("org.kde.bigscreen.music", "Main");

    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    // Bring the existing window forward when the app is launched again
    QObject::connect(&service, &KDBusService::activateRequested, &engine, [&engine]() {
        if (auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst())) {
            window->show();
            window->raise();
            window->requestActivate();
        }
    });

    return application.exec();
}
