/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "screenpool.h"

// local
#include <config-latte.h>
#include "primaryoutputwatcher.h"

// Qt
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QScreen>

// KDE
#include <KLocalizedString>
#include <KWindowSystem>

// X11
#if HAVE_X11
#include <QtX11Extras/QX11Info>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xcb_event.h>
#endif

namespace Latte {

const int ScreenPool::FIRSTSCREENID;

ScreenPool::ScreenPool(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent),
      m_configGroup(KConfigGroup(config, QStringLiteral("ScreenConnectors"))),
      m_primaryWatcher(new PrimaryOutputWatcher(this))
{
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
    m_screensTable.clear();

    //restore the known ids to connector mappings
    for (const QString &key : m_configGroup.keyList()) {
        if (key.toInt() <= 0) {
            continue;
        }

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

    if (KWindowSystem::isPlatformX11()) {
        connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &ScreenPool::primaryScreenChanged, Qt::UniqueConnection);
    }

    connect(m_primaryWatcher, &PrimaryOutputWatcher::primaryOutputNameChanged, this, &ScreenPool::onPrimaryOutputNameChanged, Qt::UniqueConnection);
}

void ScreenPool::onPrimaryOutputNameChanged(const QString &oldOutputName, const QString &newOutputName)
{
    Q_UNUSED(oldOutputName);
    Q_UNUSED(newOutputName);

    emit primaryScreenChanged(m_primaryWatcher->primaryScreen());
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

    emit screenGeometryChanged();
}


Latte::Data::ScreensTable ScreenPool::screensTable()
{   
    return m_screensTable;
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

void ScreenPool::removeScreens(const Latte::Data::ScreensTable &obsoleteScreens)
{
    for (int i=0; i<obsoleteScreens.rowCount(); ++i) {
        if (!m_screensTable.containsId(obsoleteScreens[i].id)) {
            return;
        }

        m_screensTable.remove(obsoleteScreens[i].id);
        m_configGroup.deleteEntry(obsoleteScreens[i].id);
    }
}

int ScreenPool::primaryScreenId() const
{
    return id(primaryScreen()->name());
}

QList<int> ScreenPool::secondaryScreenIds() const
{
    QList<int> secondaryscreens;

    QScreen *primary{primaryScreen()};

    for (const auto scr : qGuiApp->screens()) {
        if (scr == primary) {
            continue;
        }

        secondaryscreens << id(scr->name());
    }

    return secondaryscreens;
}

void ScreenPool::save()
{
    QMap<int, QString>::const_iterator i;

    for (int i=0; i<m_screensTable.rowCount(); ++i) {
        Data::Screen screenRecord = m_screensTable[i];
        if (screenRecord.id.toInt() >= FIRSTSCREENID) {
            m_configGroup.writeEntry(screenRecord.id, screenRecord.serialize());
        }
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
    screenRecord.id = QString::number(firstAvailableId());
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
    return screenId.isEmpty() ? NOSCREENID : screenId.toInt();
}

QString ScreenPool::connector(int id) const
{   
    QString idStr = QString::number(id);
    return (m_screensTable.containsId(idStr) ? m_screensTable[idStr].name : QString());
}

int ScreenPool::firstAvailableId() const
{
    //start counting from 10, first numbers will be used for special cases e.g. primaryScreen, id=0
    int availableId = FIRSTSCREENID;

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

QScreen *ScreenPool::primaryScreen() const
{
    return m_primaryWatcher->primaryScreen();
}

QScreen *ScreenPool::screenForId(int id)
{
    const auto screens = qGuiApp->screens();
    QScreen *screen{primaryScreen()};

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

}
