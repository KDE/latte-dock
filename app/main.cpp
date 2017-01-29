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

#include <memory>

#include <QApplication>
#include <QDebug>
#include <QQuickWindow>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

#include <KLocalizedString>
#include <KAboutData>

//! COLORS
#define CNORMAL  "\e[0m"
#define CIGREEN  "\e[1;32m"
#define CGREEN   "\e[0;32m"
#define CICYAN   "\e[1;36m"
#define CCYAN    "\e[0;36m"
#define CIRED    "\e[1;31m"
#define CRED     "\e[0;31m"

inline void configureAboutData();

void noMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
    Q_UNUSED(msg);
}

int main(int argc, char **argv)
{
    QQuickWindow::setDefaultAlphaBuffer(true);
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("latte-dock");
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("latte-dock")));
    configureAboutData();

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

    //  qputenv("QT_QUICK_CONTROLS_1_STYLE", "Desktop");
    if (!app.arguments().contains(QLatin1String("--debug"))) {
        qInstallMessageHandler(noMessageOutput);
    }

    Latte::DockCorona corona;
    return app.exec();
}

inline void configureAboutData()
{
    KAboutData about(QStringLiteral("lattedock")
                     , QStringLiteral("Latte Dock")
                     , QStringLiteral(LATTE_VERSION)
                     , i18n("Dock panel")
                     , KAboutLicense::GPL_V2
                     , QStringLiteral("\251 2016-2017 Michail Vourlakos, Smith AR"));

    about.setBugAddress(BUG_ADDRESS);
    about.setProgramLogo(QIcon::fromTheme(QStringLiteral("latte-dock")));
    about.setDesktopFileName(QStringLiteral("latte-dock"));

    // Authors
    about.addAuthor(QStringLiteral("Michail Vourlakos"), QString(), QStringLiteral("mvourlakos@gmail.com"));
    about.addAuthor(QStringLiteral("Smith AR"), QString(), QStringLiteral("audoban@openmailbox.org"));

    // Credits
    about.addCredit(QStringLiteral("varlesh"), i18n("Logo and Icons")
                    , QString(), QStringLiteral("https://github.com/varlesh"));
    about.addCredit(QStringLiteral("JenaPlinsky"), i18n("Many bug reports")
                    , QString(), QStringLiteral("https://github.com/JenaPlinsky"));
    about.addCredit(QStringLiteral("elav"), i18n("Reviews for Latte Dock, CandilDock and NowDock")
                    , QString(), QStringLiteral("https://github.com/elav"));

    // Translators
    about.setTranslator(QStringLiteral(TRANSLATORS), QStringLiteral(TRANSLATORS_EMAIL));

    KAboutData::setApplicationData(about);
}
