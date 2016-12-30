#include "dockcorona.h"

#include <memory>

#include <QApplication>
#include <QQuickWindow>
#include <QCommandLineParser>
#include <QCommandLineOption>
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
                           CIGREEN "[%{type} " CGREEN "%{time h:mm:ss.zz}" CIGREEN "]" CNORMAL
#ifndef QT_NO_DEBUG
                           CIRED " [" CCYAN "%{function}" CIRED ":" CCYAN "%{line}" CIRED "]"
#endif
                           CICYAN " - " CNORMAL "%{message}"
                           CIRED "%{if-fatal}\n%{backtrace depth=" DEPTH " separator=\"\n\"}%{endif}"
                           "%{if-warning}\n%{backtrace depth=" DEPTH " separator=\"\n\"}%{endif}"
                           "%{if-critical}\n%{backtrace depth=" DEPTH " separator=\"\n\"}%{endif}" CNORMAL));
                           
    //  qputenv("QT_QUICK_CONTROLS_1_STYLE", "Desktop");
    Latte::DockCorona corona;
    
    return app.exec();
}
