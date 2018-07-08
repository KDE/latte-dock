/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dockcorona.h"
#include "config-latte.h"
#include "importer.h"
#include "../liblattedock/dock.h"

#include <memory>
#include <csignal>

#include <QApplication>
#include <QDebug>
#include <QQuickWindow>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QDir>
#include <QLockFile>
#include <QSharedMemory>

#include <KCrash>
#include <KLocalizedString>
#include <KAboutData>
#include <KDBusService>
#include <KQuickAddons/QtQuickSettings>


//! COLORS
#define CNORMAL  "\e[0m"
#define CIGREEN  "\e[1;32m"
#define CGREEN   "\e[0;32m"
#define CICYAN   "\e[1;36m"
#define CCYAN    "\e[0;36m"
#define CIRED    "\e[1;31m"
#define CRED     "\e[0;31m"

inline void configureAboutData();
inline void detectPlatform(int argc, char **argv);

int main(int argc, char **argv)
{
    //Plasma scales itself to font DPI
    //on X, where we don't have compositor scaling, this generally works fine.
    //also there are bugs on older Qt, especially when it comes to fractional scaling
    //there's advantages to disabling, and (other than small context menu icons) few advantages in enabling

    //On wayland, it's different. Everything is simpler as all co-ordinates are in the same co-ordinate system
    //we don't have fractional scaling on the client so don't hit most the remaining bugs and
    //even if we don't use Qt scaling the compositor will try to scale us anyway so we have no choice
    if (!qEnvironmentVariableIsSet("PLASMA_USE_QT_SCALING")) {
        qunsetenv("QT_DEVICE_PIXEL_RATIO");
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    } else {
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }

    QQuickWindow::setDefaultAlphaBuffer(true);

    const bool qpaVariable = qEnvironmentVariableIsSet("QT_QPA_PLATFORM");
    detectPlatform(argc, argv);
    QApplication app(argc, argv);

    if (!qpaVariable) {
        // don't leak the env variable to processes we start
        qunsetenv("QT_QPA_PLATFORM");
    }

    KQuickAddons::QtQuickSettings::init();

    KLocalizedString::setApplicationDomain("latte-dock");
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("latte-dock")));
    //protect from closing app when changing to "alternative session" and back
    app.setQuitOnLastWindowClosed(false);

    configureAboutData();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {{"r", "replace"}, i18nc("command line", "Replace the current Latte instance.")}
        , {{"d", "debug"}, i18nc("command line", "Show the debugging messages on stdout.")}
        , {"default-layout", i18nc("command line", "Import and load default layout on startup.")}
        , {"available-layouts", i18nc("command line", "Print available layouts")}
        , {"layout", i18nc("command line", "Load specific layout on startup."), i18nc("command line: load", "layout_name")}
        , {"import-layout", i18nc("command line", "Import and load a layout."), i18nc("command line: import", "file_name")}
        , {"import-full", i18nc("command line", "Import full configuration."), i18nc("command line: import", "file_name")}
        , {"single", i18nc("command line", "Single layout memory mode. Only one layout is active at any case.")}
        , {"multiple", i18nc("command line", "Multiple layouts memory mode. Multiple layouts can be active at any time based on Activities running.")}
    });

    //! START: Hidden options for Developer and Debugging usage
    QCommandLineOption graphicsOption(QStringList() << QStringLiteral("graphics"));
    graphicsOption.setDescription(QStringLiteral("Draw boxes around of the applets."));
    graphicsOption.setHidden(true);
    parser.addOption(graphicsOption);

    QCommandLineOption withWindowOption(QStringList() << QStringLiteral("with-window"));
    withWindowOption.setDescription(QStringLiteral("Open a window with much debug information"));
    withWindowOption.setHidden(true);
    parser.addOption(withWindowOption);

    QCommandLineOption maskOption(QStringList() << QStringLiteral("mask"));
    maskOption.setDescription(QStringLiteral("Show messages of debugging for the mask (Only useful to devs)."));
    maskOption.setHidden(true);
    parser.addOption(maskOption);

    QCommandLineOption timersOption(QStringList() << QStringLiteral("timers"));
    timersOption.setDescription(QStringLiteral("Show messages for debugging the timers (Only useful to devs)."));
    timersOption.setHidden(true);
    parser.addOption(timersOption);

    QCommandLineOption spacersOption(QStringList() << QStringLiteral("spacers"));
    spacersOption.setDescription(QStringLiteral("Show visual indicators for debugging spacers (Only useful to devs)."));
    spacersOption.setHidden(true);
    parser.addOption(spacersOption);

    QCommandLineOption overloadedIconsOption(QStringList() << QStringLiteral("overloaded-icons"));
    overloadedIconsOption.setDescription(QStringLiteral("Show visual indicators for debugging overloaded applets icons (Only useful to devs)."));
    overloadedIconsOption.setHidden(true);
    parser.addOption(overloadedIconsOption);
    //! END: Hidden options

    parser.process(app);

    if (parser.isSet(QStringLiteral("available-layouts"))) {
        QStringList layouts = Latte::Importer::availableLayouts();

        if (layouts.count() > 0) {
            qInfo() << i18n("Available layouts that can be used to start Latte:");

            foreach (auto layout, layouts) {
                qInfo() << "     " << layout;
            }
        } else {
            qInfo() << i18n("There are no available layouts, during startup Default will be used.");
        }

        qGuiApp->exit();
        return 0;
    }

    bool defaultLayoutOnStartup = false;
    int memoryUsage = -1;
    QString layoutNameOnStartup = "";

    if (parser.isSet(QStringLiteral("default-layout"))) {
        defaultLayoutOnStartup = true;
    } else if (parser.isSet(QStringLiteral("layout"))) {
        layoutNameOnStartup = parser.value(QStringLiteral("layout"));

        if (!Latte::Importer::layoutExists(layoutNameOnStartup)) {
            qInfo() << i18nc("layout missing", "This layout doesnt exist in the system.");
            qGuiApp->exit();
            return 0;
        }
    }

    QString username = qgetenv("USER");

    if (username.isEmpty())
        username = qgetenv("USERNAME");

    QLockFile lockFile {QDir::tempPath() + "/latte-dock." + username + ".lock"};

    int timeout {100};

    if (parser.isSet(QStringLiteral("replace")) || parser.isSet(QStringLiteral("import-full"))) {
        qint64 pid{ -1};

        if (lockFile.getLockInfo(&pid, nullptr, nullptr)) {
            kill(static_cast<pid_t>(pid), SIGINT);
            timeout = 3000;
        }
    }

    if (!lockFile.tryLock(timeout)) {
        qInfo() << i18n("An instance is already running!, use --replace to restart Latte");
        qGuiApp->exit();
        return 0;
    }

    if (parser.isSet(QStringLiteral("import-full"))) {
        bool imported = Latte::Importer::importHelper(parser.value(QStringLiteral("import-full")));

        if (!imported) {
            qInfo() << i18n("The configuration cannot be imported");
            qGuiApp->exit();
            return 0;
        }
    }

    if (parser.isSet(QStringLiteral("import-layout"))) {
        QString importedLayout = Latte::Importer::importLayoutHelper(parser.value(QStringLiteral("import-layout")));

        if (importedLayout.isEmpty()) {
            qInfo() << i18n("The layout cannot be imported");
            qGuiApp->exit();
            return 0;
        } else {
            layoutNameOnStartup = importedLayout;
        }
    }

    if (parser.isSet(QStringLiteral("multiple"))) {
        memoryUsage = (int)(Latte::Dock::MultipleLayouts);
    } else if (parser.isSet(QStringLiteral("single"))) {
        memoryUsage = (int)(Latte::Dock::SingleLayout);
    }

    if (parser.isSet(QStringLiteral("debug")) || parser.isSet(QStringLiteral("mask"))) {
        //! set pattern for debug messages
        //! [%{type}] [%{function}:%{line}] - %{message} [%{backtrace}]

        qSetMessagePattern(QStringLiteral(
                               CIGREEN "[%{type} " CGREEN "%{time h:mm:ss.zz}" CIGREEN "]" CNORMAL
#ifndef QT_NO_DEBUG
                               CIRED " [" CCYAN "%{function}" CIRED ":" CCYAN "%{line}" CIRED "]"
#endif
                               CICYAN " - " CNORMAL "%{message}"
                               CIRED "%{if-fatal}\n%{backtrace depth=8 separator=\"\n\"}%{endif}"
                               "%{if-critical}\n%{backtrace depth=8 separator=\"\n\"}%{endif}" CNORMAL));
    } else {
        const auto noMessageOutput = [](QtMsgType, const QMessageLogContext &, const QString &) {};
        qInstallMessageHandler(noMessageOutput);
    }


    auto signal_handler = [](int) {
        qGuiApp->exit();
    };

    std::signal(SIGKILL, signal_handler);
    std::signal(SIGINT, signal_handler);

    KCrash::setDrKonqiEnabled(true);
    KCrash::setFlags(KCrash::AutoRestart | KCrash::AlwaysDirectly);

    Latte::DockCorona corona(defaultLayoutOnStartup, layoutNameOnStartup, memoryUsage);
    KDBusService service(KDBusService::Unique);

    return app.exec();
}

inline void configureAboutData()
{
    KAboutData about(QStringLiteral("lattedock")
                     , QStringLiteral("Latte Dock")
                     , QStringLiteral(VERSION)
                     , i18n("Latte is a dock based on plasma frameworks that provides an elegant and "
                            "intuitive experience for your tasks and plasmoids. It animates its contents "
                            "by using parabolic zoom effect and tries to be there only when it is needed."
                            "\n\n\"Art in Coffee\"")
                     , KAboutLicense::GPL_V2
                     , QStringLiteral("\251 2016-2017 Michail Vourlakos, Smith AR"));

    about.setHomepage(WEBSITE);
    about.setBugAddress(BUG_ADDRESS);
    about.setProgramLogo(QIcon::fromTheme(QStringLiteral("latte-dock")));
    about.setDesktopFileName(QStringLiteral("latte-dock"));

    // Authors
    about.addAuthor(QStringLiteral("Michail Vourlakos"), QString(), QStringLiteral("mvourlakos@gmail.com"));
    about.addAuthor(QStringLiteral("Smith AR"), QString(), QStringLiteral("audoban@openmailbox.org"));

    KAboutData::setApplicationData(about);
}

//! used the version provided by PW:KWorkspace
inline void detectPlatform(int argc, char **argv)
{
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        return;
    }

    for (int i = 0; i < argc; i++) {
        if (qstrcmp(argv[i], "-platform") == 0 ||
            qstrcmp(argv[i], "--platform") == 0 ||
            QByteArray(argv[i]).startsWith("-platform=") ||
            QByteArray(argv[i]).startsWith("--platform=")) {
            return;
        }
    }

    const QByteArray sessionType = qgetenv("XDG_SESSION_TYPE");

    if (sessionType.isEmpty()) {
        return;
    }

    if (qstrcmp(sessionType, "wayland") == 0) {
        qputenv("QT_QPA_PLATFORM", "wayland");
    } else if (qstrcmp(sessionType, "x11") == 0) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
}
