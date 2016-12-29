/*
 * Copyright 2014  Bhushan Shah <bhush94@gmail.com>
 * Copyright 2014 Marco Martin <notmart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "nowdockcorona.h"

#include <memory>

#include <QApplication>
#include <QQuickWindow>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>
#include <QDebug>

#include <KLocalizedString>

//! COLORS
#define CNORMAL  "\e[0m"
#define CIGREEN  "\e[1;32m"
#define CGREEN   "\e[0;32m"
#define CICYAN   "\e[1;36m"
#define CCYAN    "\e[0;36m"
#define CIRED    "\e[1;31m"
#define CRED     "\e[0;31m"

#ifdef QT_NO_DEBUG
    #define DEPTH "1"
#else
    #define DEPTH "8"
#endif

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    QQuickWindow::setDefaultAlphaBuffer(true);
    
    QApplication app(argc, argv);
    app.setApplicationVersion(version);
    
    app.setOrganizationDomain(QStringLiteral("latte-dock"));
    KLocalizedString::setApplicationDomain("latte-dock");
    app.setApplicationName(QStringLiteral("Latte Dock"));
    
    //! set pattern for debug messages
    //! [%{type}] [%{function}:%{line}] - %{message} [%{backtrace}]
    qSetMessagePattern(QStringLiteral(
                           CIGREEN "[%{type} " CGREEN "%{time h:mm:ss.zzzz}" CIGREEN "]" CNORMAL
#ifndef QT_NO_DEBUG
                           CIRED " [" CCYAN "%{function}" CIRED ":" CCYAN "%{line}" CIRED "]"
#endif
                           CICYAN " - " CNORMAL "%{message}"
                           CIRED "%{if-fatal}\n%{backtrace depth=" DEPTH " separator=\"\n\"}%{endif}"
                           "%{if-warning}\n%{backtrace depth=" DEPTH " separator=\"\n\"}%{endif}"
                           "%{if-critical}\n%{backtrace depth=" DEPTH " separator=\"\n\"}%{endif}" CNORMAL));
                           
    //  qputenv("QT_QUICK_CONTROLS_1_STYLE", "Desktop");
    NowDockCorona corona;
    
    return app.exec();
}
