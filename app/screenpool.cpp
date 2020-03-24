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
}

void ScreenPool::load()
{
    m_primaryConnector = QString();
    m_connectorForId.clear();
    m_idForConnector.clear();

    QScreen *primary = qGuiApp->primaryScreen();

    if (primary) {
        m_primaryConnector = primary->name();

        if (!m_primaryConnector.isEmpty()) {
            //m_connectorForId[0] = m_primaryConnector;
            //m_idForConnector[m_primaryConnector] = 0;
        }
    }

    //restore the known ids to connector mappings
    for (const QString &key : m_configGroup.keyList()) {
        QString connector = m_configGroup.readEntry(key, QString());
        qDebug() << "connector :" << connector << " - " << key;

        if (!key.isEmpty() && !connector.isEmpty() &&
            !m_connectorForId.contains(key.toInt()) &&
            !m_idForConnector.contains(connector)) {
            m_connectorForId[key.toInt()] = connector;
            m_idForConnector[connector] = key.toInt();
            qDebug() << "Known Screen - " << connector << " - " << key.toInt();
        } else if (m_idForConnector.value(connector) != key.toInt()) {
            m_configGroup.deleteEntry(key);
        }
    }

    // if there are already connected unknown screens, map those
    // all needs to be populated as soon as possible, otherwise
    // containment->screen() will return an incorrect -1
    // at startup, if it' asked before corona::addOutput()
    // is performed, driving to the creation of a new containment
    for (QScreen *screen : qGuiApp->screens()) {
        if (!m_idForConnector.contains(screen->name())) {
            insertScreenMapping(firstAvailableId(), screen->name());
        }
    }
}

ScreenPool::~ScreenPool()
{
    m_configGroup.sync();
}


QString ScreenPool::reportHtml(const QList<int> &assignedScreens) const
{
    QString report;

    report += "<table cellspacing='8'>";
    report += "<tr><td align='center'><b>" + i18nc("screen id","ID") + "</b></td>" +
            "<td align='center'><b>" + i18nc("screen name", "Name") + "</b></td>" +
            "<td align='center'><b>" + i18nc("screen type", "Type") + "</b></td>" +
            "<td align='center'><b>" + i18n("Docks/Panels") + "</b></td></tr>";

    report += "<tr><td colspan='4'><hr></td></tr>";

    for(const QString &connector : m_connectorForId) {
        int scrId = id(connector);
        bool hasViews = assignedScreens.contains(scrId);
        bool primary = m_primaryConnector == connector;
        bool secondary = !primary && screenExists(scrId);

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

    report += "</table>";

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

QString ScreenPool::primaryConnector() const
{
    return m_primaryConnector;
}

void ScreenPool::setPrimaryConnector(const QString &primary)
{
    //the ":" check fixes the strange plasma/qt issues when changing layouts
    //there are case that the QScreen instead of the correct screen name
    //returns "0:0", this check prevents from breaking the screens database
    //from garbage ids
    if ((m_primaryConnector == primary) || primary.startsWith(":")) {
        return;
    }

    Q_ASSERT(m_idForConnector.contains(primary));

    /* int oldIdForPrimary = m_idForConnector.value(primary);

     m_idForConnector[primary] = 0;
     m_connectorForId[0] = primary;
     m_idForConnector[m_primaryConnector] = oldIdForPrimary;
     m_connectorForId[oldIdForPrimary] = m_primaryConnector;
     m_primaryConnector = primary;
    */
    save();
}

void ScreenPool::save()
{
    QMap<int, QString>::const_iterator i;

    for (i = m_connectorForId.constBegin(); i != m_connectorForId.constEnd(); ++i) {
        m_configGroup.writeEntry(QString::number(i.key()), i.value());
    }

    //write to disck every 30 seconds at most
    m_configSaveTimer.start(30000);
}

void ScreenPool::insertScreenMapping(int id, const QString &connector)
{
    //Q_ASSERT(!m_connectorForId.contains(id) || m_connectorForId.value(id) == connector);
    //Q_ASSERT(!m_idForConnector.contains(connector) || m_idForConnector.value(connector) == id);

    //the ":" check fixes the strange plasma/qt issues when changing layouts
    //there are case that the QScreen instead of the correct screen name
    //returns "0:0", this check prevents from breaking the screens database
    //from garbage ids
    if (connector.startsWith(":"))
        return;

    qDebug() << "add connector..." << connector;

    if (id == 0) {
        m_primaryConnector = connector;
    } else {
        m_connectorForId[id] = connector;
        m_idForConnector[connector] = id;
    }

    save();
}

int ScreenPool::id(const QString &connector) const
{
    if (!m_idForConnector.contains(connector)) {
        return -1;
    }

    return m_idForConnector.value(connector);
}

QString ScreenPool::connector(int id) const
{   
    Q_ASSERT(m_connectorForId.contains(id));

    return m_connectorForId.value(id);
}

int ScreenPool::firstAvailableId() const
{
    //start counting from 10, first numbers will
    //be used for special cases.
    //e.g primaryScreen, id=0
    int i = 10;

    //find the first integer not stored in m_connectorForId
    //m_connectorForId is the only map, so the ids are sorted
    for (const int &existingId : m_connectorForId.keys()) {
        if (i != existingId) {
            return i;
        }

        ++i;
    }

    return i;
}

QList <int> ScreenPool::knownIds() const
{
    return m_connectorForId.keys();
}

bool ScreenPool::hasId(int id) const
{
    return ((id!=-1) &&  knownIds().contains(id));
}

bool ScreenPool::screenExists(int id) const
{
    if (id != -1 && knownIds().contains(id)) {
        QString scrName = connector(id);

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

    if (id != -1 && knownIds().contains(id)) {
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
        if (qGuiApp->primaryScreen()->name() != primaryConnector()) {
            //new screen?
            if (id(qGuiApp->primaryScreen()->name()) < 0) {
                insertScreenMapping(firstAvailableId(), qGuiApp->primaryScreen()->name());
            }

            //switch the primary screen in the pool
            setPrimaryConnector(qGuiApp->primaryScreen()->name());

            emit primaryPoolChanged();
        }
    }

#endif
    return false;
}

}

#include "moc_screenpool.cpp"
