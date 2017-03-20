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
#include "globalsettings.h"

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

#include <KLocalizedString>
#include <KAboutData>
#include <KDBusService>


//! COLORS
#define CNORMAL  "\e[0m"
#define CIGREEN  "\e[1;32m"
#define CGREEN   "\e[0;32m"
#define CICYAN   "\e[1;36m"
#define CCYAN    "\e[0;36m"
#define CIRED    "\e[1;31m"
#define CRED     "\e[0;31m"

inline void configureAboutData();

int main(int argc, char **argv)
{
    //    Devive pixel ratio has some problems in latte (plasmashell) currently.
    //     - dialog continually expands (347951)
    //     - Text element text is screwed (QTBUG-42606)
    //     - Panel struts (350614)
    //  This variable should possibly be removed when all are fixed
    qunsetenv("QT_DEVICE_PIXEL_RATIO");
    //  qputenv("QT_QUICK_CONTROLS_1_STYLE", "Desktop");
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

    QQuickWindow::setDefaultAlphaBuffer(true);
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("latte-dock");
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("latte-dock")));

    configureAboutData();

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({
        {{"r", "replace"}, i18nc("command line", "Replace the current dock instance.")}
        , {{"d", "debug"}, i18nc("command line", "Show the debugging messages on stdout.")}
        , {"mask", i18nc("command line" , "Show messages of debugging for the mask (Only useful to devs).")}
        , {"graphics", i18nc("command line", "Draw boxes around of the applets.")}
        , {"with-window", i18nc("command line", "Open a window with much debug information.")}
        , {"import", i18nc("command line", "Import configuration."), i18nc("command line: import", "file_name")}
    });

    parser.process(app);

    QLockFile lockFile {QDir::tempPath() + "/latte-dock.lock"};

    int timeout {100};
    if (parser.isSet(QStringLiteral("replace")) || parser.isSet(QStringLiteral("import"))) {
        qint64 pid{-1};
        if (lockFile.getLockInfo(&pid, nullptr, nullptr)) {
            kill(static_cast<pid_t>(pid), SIGINT);
            timeout = 3000;
        }
    }

    if (!lockFile.tryLock(timeout)) {
        qInfo() << i18n("An instance is already running!, use --replace to restart Latte");
        qGuiApp->exit();
    }

    if (parser.isSet(QStringLiteral("import"))) {
        bool imported = Latte::GlobalSettings::importHelper(parser.value(QStringLiteral("import")));

        if (!imported) {
            qInfo() << i18n("The configuration cannot be imported");
            app.quit();
        }
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

    Latte::DockCorona corona;
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
                            "by using parabolic zoom effect and trys to be there only when it is needed."
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

    // Credits
    about.addCredit(QStringLiteral("Alexey Varfolomeev (varlesh)"), i18n("Logo and Icons")
                    , QString(), QStringLiteral("https://github.com/varlesh"));
    about.addCredit(QStringLiteral("Ivan Bordoni"), i18n("Many bug reports")
                    , QString(), QStringLiteral("https://github.com/JenaPlinsky"));
    about.addCredit(QStringLiteral("Ernesto Acosta (elav)"), i18n("Reviews for Latte Dock, CandilDock and NowDock")
                    , QString(), QStringLiteral("https://github.com/elav"));

    // Translators
    about.setTranslator(QStringLiteral(TRANSLATORS), QStringLiteral(TRANSLATORS_EMAIL));

    KAboutData::setApplicationData(about);
}
