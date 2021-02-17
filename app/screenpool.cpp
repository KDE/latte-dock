/*
 *  Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "screenpool.h"

// local
#include <config-latte.h>

// Qt
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QScreen>

// KDE
#include <KLocalizedString>

// X11
#if HAVE_X11
#include <QtX11Extras/QX11Info>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xcb_event.h>
#endif

namespace Latte {

ScreenPool::ScreenPool(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent),
      m_configGroup(KConfigGroup(config, QStringLiteral("ScreenConnectors")))
{
    qApp->installNativeEventFilter(this);

    m_configSaveTimer.setSingleShot(true);
    connect(&m_configSaveTimer, &QTimer::timeout, this, [this]() {
        m_configGroup.sync();
    });

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &ScreenPool::onScreenAdded, Qt::UniqueConnection);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &ScreenPool::onScreenRemoved, Qt::UniqueConnection);
}

ScreenPool::~ScreenPool()
{
    m_configGroup.sync();
}

void ScreenPool::load()
{
    m_lastPrimaryConnector = QString();
    m_screensTable.clear();

    QScreen *primary = qGuiApp->primaryScreen();

    if (primary) {
        m_lastPrimaryConnector = primary->name();
    }

    //restore the known ids to connector mappings
    for (const QString &key : m_configGroup.keyList()) {
        QString serialized = m_configGroup.readEntry(key, QString());

        Data::Screen screenRecord(key, serialized);
        //qDebug() << "org.kde.latte ::: " << screenRecord.id << ":" << screenRecord.serialize();

        if (!key.isEmpty() && !serialized.isEmpty() && !m_screensTable.containsId(key)) {
            m_screensTable << screenRecord;
            qDebug() << "org.kde.latte :: Known Screen - " << screenRecord.id << " : " << screenRecord.name << " : " << screenRecord.geometry;
        }
    }

    // if there are already connected unknown screens, map those
    // all needs to be populated as soon as possible, otherwise
    // containment->screen() will return an incorrect -1
    // at startup, if it' asked before corona::addOutput()
    // is performed, driving to the creation of a new containment
    for (QScreen *screen : qGuiApp->screens()) {
        onScreenRemoved(screen);

        if (!m_screensTable.containsName(screen->name())) {
            insertScreenMapping(screen->name());
        } else {
            updateScreenGeometry(screen);
        }

        onScreenAdded(screen);
    }
}

void ScreenPool::onScreenAdded(const QScreen *screen)
{
    connect(screen, &QScreen::geometryChanged, this, [&, screen]() {
        updateScreenGeometry(screen);
    });
}

void ScreenPool::onScreenRemoved(const QScreen *screen)
{
    disconnect(screen, &QScreen::geometryChanged, this, nullptr);
}

void ScreenPool::updateScreenGeometry(const QScreen *screen)
{
    if (!screen) {
        return;
    }

    if (m_screensTable.containsName(screen->name())) {
        updateScreenGeometry(id(screen->name()), screen->geometry());
    }
}

void ScreenPool::updateScreenGeometry(const int &screenId, const QRect &screenGeometry)
{
    QString scrIdStr = QString::number(screenId);

    if (!m_screensTable.containsId(scrIdStr) || m_screensTable[scrIdStr].geometry == screenGeometry) {
        return;
    }

    m_screensTable[scrIdStr].geometry = screenGeometry;
    save();
}


QString ScreenPool::reportHtml(const QList<int> &assignedScreens) const
{
    QString report;

    /* report += "<table cellspacing='8'>";
    report += "<tr><td align='center'><b>" + i18nc("screen id","ID") + "</b></td>" +
            "<td align='center'><b>" + i18nc("screen name", "Name") + "</b></td>" +
            "<td align='center'><b>" + i18nc("screen type", "Type") + "</b></td>" +
            "<td align='center'><b>" + i18n("Docks/Panels") + "</b></td></tr>";

    report += "<tr><td colspan='4'><hr></td></tr>";

    for(const QString &connector : m_connectorForId) {
        int scrId = id(connector);
        bool hasViews = assignedScreens.contains(scrId);
        bool primary = m_lastPrimaryConnector == connector;
        bool secondary = !primary && isScreenActive(scrId);

        report += "<tr>";

        //! ScreenId
        QString idStr = "[" + QString::number(scrId) + "]";
        if (primary || secondary) {
            idStr = "<b>" + idStr +"</b>";
        }
        report += "<td align='center'>" + idStr + "</td>";

        //! ScreenName and Primary flag
        QString connectorStr = connector;
        if (primary || secondary) {
            connectorStr = "<b>"+ connector + "</b>";
        }
        report += "<td align='center'>" + connectorStr + "</td>";

        //! ScreenType
        QString typeStr = "";
        if (primary) {
            typeStr = "<b><font color='green'>[" + i18nc("primary screen","Primary") + "]</font></b>";
        } else if (secondary) {
            typeStr = "<b>[" + i18nc("secondary screen","Secondary") + "]</b>";
        } else {
            typeStr = "<i>" + i18nc("inactive screen","inactive") + "</i>";
        }

        report += "<td align='center'>" + typeStr +"</td>";

        //! Screen has not assigned any docks/panels
        QString notAssignedStr = "";
        if (!hasViews) {
            notAssignedStr = "<font color='red'><i>[" + i18nc("it has not latte docks/panels", "none") + "]</i></font>";
        } else {
            notAssignedStr = " ✔ ";
        }

        report += "<td align='center'>" + notAssignedStr +"</td>";

        report += "</tr>";
    }

    report += "</table>";*/

    return report;
}

void ScreenPool::reload(QString path)
{
    QFile rcfile(QString(path + "/lattedockrc"));

    if (rcfile.exists()) {
        qDebug() << "load screen connectors from ::: " << rcfile.fileName();
        KSharedConfigPtr newFile = KSharedConfig::openConfig(rcfile.fileName());
        m_configGroup = KConfigGroup(newFile, QStringLiteral("ScreenConnectors"));
        load();
    }
}

int ScreenPool::primaryScreenId() const
{
    return id(qGuiApp->primaryScreen()->name());
}

void ScreenPool::save()
{
    QMap<int, QString>::const_iterator i;

    for (int i=0; i<m_screensTable.rowCount(); ++i) {
        Data::Screen screenRecord = m_screensTable[i];
        m_configGroup.writeEntry(screenRecord.id, screenRecord.serialize());
    }

    //write to disck every 10 seconds at most
    m_configSaveTimer.start(10000);
}

void ScreenPool::insertScreenMapping(const QString &connector)
{
    //the ":" check fixes the strange plasma/qt issues when changing layouts
    //there are case that the QScreen instead of the correct screen name
    //returns "0:0", this check prevents from breaking the screens database
    //from garbage ids
    if (m_screensTable.containsName(connector) || connector.startsWith(":")) {
        return;
    }

    qDebug() << "add connector..." << connector;

    Data::Screen screenRecord;
    screenRecord.id = firstAvailableId();
    screenRecord.name = connector;

    //! update screen geometry
    for (const auto scr : qGuiApp->screens()) {
        if (scr->name() == connector) {
            screenRecord.geometry = scr->geometry();
            break;
        }
    }

    m_screensTable << screenRecord;
    save();
}

int ScreenPool::id(const QString &connector) const
{
    QString screenId = m_screensTable.idForName(connector);
    return screenId.isEmpty() ? -1 : screenId.toInt();
}

QString ScreenPool::connector(int id) const
{   
    QString idStr = QString::number(id);
    return (m_screensTable.containsId(idStr) ? m_screensTable[idStr].name : QString());
}

int ScreenPool::firstAvailableId() const
{
    //start counting from 10, first numbers will be used for special cases e.g. primaryScreen, id=0
    int availableId = 10;

    for (int row=0; row<m_screensTable.rowCount(); ++row) {
        if (!m_screensTable.containsId(QString::number(availableId))) {
            return availableId;
        }

        availableId++;
    }

    return availableId;
}

bool ScreenPool::hasScreenId(int screenId) const
{
    return ((screenId>=0) && m_screensTable.containsId(QString::number(screenId)));
}

bool ScreenPool::isScreenActive(int screenId) const
{
    if (hasScreenId(screenId)) {
        QString scrName = connector(screenId);

        for (const auto scr : qGuiApp->screens()) {
            if (scr->name() == scrName) {
                return true;
            }
        }
    }

    return false;
}

QScreen *ScreenPool::screenForId(int id)
{
    const auto screens = qGuiApp->screens();
    QScreen *screen{qGuiApp->primaryScreen()};

    if (hasScreenId(id)) {
        QString scrName = connector(id);

        for (const auto scr : screens) {
            if (scr->name() == scrName) {
                return scr;
            }
        }
    }

    return screen;
}

bool ScreenPool::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
{
    Q_UNUSED(result);
#if HAVE_X11

    // a particular edge case: when we switch the only enabled screen
    // we don't have any signal about it, the primary screen changes but we have the same old QScreen* getting recycled
    // see https://bugs.kde.org/show_bug.cgi?id=373880
    // if this slot will be invoked many times, their//second time on will do nothing as name and primaryconnector will be the same by then
    if (eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);

    const auto responseType = XCB_EVENT_RESPONSE_TYPE(ev);

    const xcb_query_extension_reply_t *reply = xcb_get_extension_data(QX11Info::connection(), &xcb_randr_id);

    if (responseType == reply->first_event + XCB_RANDR_SCREEN_CHANGE_NOTIFY) {
        if (qGuiApp->primaryScreen()->name() != m_lastPrimaryConnector) {
            //new screen?
            if (id(qGuiApp->primaryScreen()->name()) < 0) {
                insertScreenMapping(qGuiApp->primaryScreen()->name());
            }

            m_lastPrimaryConnector = qGuiApp->primaryScreen()->name();
            emit primaryPoolChanged();
        }
    }

#endif
    return false;
}

}

#include "moc_screenpool.cpp"
