// SPDX-FileCopyrightText: 2026 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "library.h"
#include "mpris.h"
#include "player.h"
#include "version.h"

#include <KAboutData>
#include <KCoreAddons>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

using namespace Qt::StringLiterals;

const QList<QUrl> urlsFromArguments(const QStringList &arguments, const QString &workingDirectory)
{
    QList<QUrl> urls;
    for (const QString &argument : arguments) {
        urls.append(QUrl::fromUserInput(argument, workingDirectory, QUrl::AssumeLocalFile));
    }
    return urls;
}

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        qputenv("QT_QUICK_CONTROLS_STYLE", "org.kde.desktop");
    }

    QGuiApplication application(argc, argv);

    KLocalizedString::setApplicationDomain("encoremusic");
    KAboutData aboutData(u"encoremusic"_s,
                         i18n("Encore Music"),
                         QStringLiteral(ENCOREMUSIC_VERSION_STRING),
                         i18n("Play your music collection"),
                         KAboutLicense::GPL_V2,
                         i18n("© 2026 KDE Community"));
    aboutData.addAuthor(i18n("Devin Lin"), i18n("Maintainer"), u"devin@kde.org"_s);
    aboutData.setLicense(KAboutLicense::GPL_V2, KAboutLicense::OrLaterVersions);
    aboutData.setHomepage(u"https://invent.kde.org/devinlin/bigscreen-music"_s);
    aboutData.setBugAddress("https://invent.kde.org/devinlin/bigscreen-music/-/issues");
    aboutData.setUrl(KAboutData::VCSBrowser, u"https://invent.kde.org/devinlin/bigscreen-music"_s);
    aboutData.setDesktopFileName(u"org.kde.encoremusic"_s);
    KAboutData::setApplicationData(aboutData);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(u"org.kde.encoremusic"_s));

    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.addPositionalArgument(u"files"_s, i18n("Audio files or folders to play"), u"[files...]"_s);
    parser.process(application);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    // Start reading the library before the UI loads so it's ready soooner
    Library *library = Library::instance();
    library->rescan();
    QObject::connect(&application, &QCoreApplication::aboutToQuit, library, &Library::cancel);

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.loadFromModule("org.kde.encoremusic", "Main");

    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    auto raiseWindow = [&engine]() {
        if (auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst())) {
            window->show();
            window->raise();
            window->requestActivate();
        }
    };

    Player *player = Player::instance();
    Mpris mpris(player);
    QObject::connect(&mpris, &Mpris::raiseRequested, &engine, raiseWindow);

    player->openUrls(urlsFromArguments(parser.positionalArguments(), QDir::currentPath()));

    QObject::connect(&service,
                     &KDBusService::activateRequested,
                     &engine,
                     [&parser, player, raiseWindow](const QStringList &arguments, const QString &workingDirectory) {
                         raiseWindow();
                         if (arguments.isEmpty()) {
                             return;
                         }
                         parser.parse(arguments);
                         player->openUrls(urlsFromArguments(parser.positionalArguments(), workingDirectory));
                     });

    return application.exec();
}
