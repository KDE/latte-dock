/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// local
#include "shortcutstracker.h"
#include "../tools/commontools.h"

// Qt
#include <QAction>
#include <QDir>
#include <QDebug>
#include <QLatin1String>

// KDE
#include <KConfigGroup>
#include <KDirWatch>
#include <KGlobalAccel>


#define GLOBALSHORTCUTSCONFIG "kglobalshortcutsrc"
#define APPLETSHORTCUTKEY "activate widget "

namespace Latte {
namespace ShortcutsPart {

ShortcutsTracker::ShortcutsTracker(QObject *parent)
    : QObject(parent)
{
    //! load global shortcuts badges at startup
    initGlobalShortcutsWatcher();
    parseGlobalShortcuts();
    clearAllAppletShortcuts();
}

ShortcutsTracker::~ShortcutsTracker()
{
}

void ShortcutsTracker::initGlobalShortcutsWatcher()
{
    for (int i=1; i<=19; ++i) {
        m_badgesForActivate << QString();
    }

    const QString globalShortcutsFilePath = Latte::configPath() + "/" + GLOBALSHORTCUTSCONFIG;
    m_shortcutsConfigPtr = KSharedConfig::openConfig(globalShortcutsFilePath);

    KDirWatch::self()->addFile(globalShortcutsFilePath);

    connect(KDirWatch::self(), &KDirWatch::dirty, this, &ShortcutsTracker::shortcutsFileChanged, Qt::QueuedConnection);
    connect(KDirWatch::self(), &KDirWatch::created, this, &ShortcutsTracker::shortcutsFileChanged, Qt::QueuedConnection);
}

bool ShortcutsTracker::basedOnPositionEnabled() const
{
    return m_basedOnPositionEnabled;
}

QStringList ShortcutsTracker::badgesForActivate() const
{
    return m_badgesForActivate;
}

void ShortcutsTracker::shortcutsFileChanged(const QString &file)
{
    if (!file.endsWith(GLOBALSHORTCUTSCONFIG)) {
        return;
    }

    m_shortcutsConfigPtr->reparseConfiguration();
    parseGlobalShortcuts();
}

QList<uint> ShortcutsTracker::appletsWithPlasmaShortcuts()
{
    return m_appletShortcuts.keys();
}

QString ShortcutsTracker::appletShortcutBadge(int appletId)
{
    if (m_appletShortcuts.contains(appletId)) {
        return m_appletShortcuts[appletId];
    }

    return QString();
}

QString ShortcutsTracker::shortcutToBadge(QStringList shortcutRecords)
{
    QString badge;

    if (shortcutRecords.count()>0 && shortcutRecords[0] != "none") {
        QStringList modifiers = shortcutRecords[0].split("+");

        if (modifiers.count() >= 1) {
            badge = modifiers[modifiers.count() - 1];

            //! when shortcut follows Meta+"Character" scheme
            if (modifiers.count() == 2 && modifiers[0] == QLatin1String("Meta")) {
                badge = badge.toLower();
            } else {
                badge = badge.toUpper();
            }
        }
    }

    return badge;
}

void ShortcutsTracker::parseGlobalShortcuts()
{
    KConfigGroup latteGroup = KConfigGroup(m_shortcutsConfigPtr, "lattedock");

    if (latteGroup.exists()) {
        m_badgesForActivate.clear();
        m_appletShortcuts.clear();

        for (int i = 1; i <= 19; ++i) {
            QString entry = "activate entry " + QString::number(i);

            if (latteGroup.hasKey(entry)) {
                QStringList records = latteGroup.readEntry(entry, QStringList());
                if (records.count() > 0) {
                    records[0] = records[0].split("\t")[0];
                }
                m_badgesForActivate << shortcutToBadge(records);
            } else {
                m_badgesForActivate << "";
            }
        }

        m_basedOnPositionEnabled = (!m_badgesForActivate[0].isEmpty() && !m_badgesForActivate[1].isEmpty());

        for(auto &key : latteGroup.keyList()) {
            if (key.startsWith(APPLETSHORTCUTKEY)) {
                QStringList records = latteGroup.readEntry(key, QStringList());
                int appletId = key.remove(APPLETSHORTCUTKEY).toInt();

                m_appletShortcuts[appletId] = shortcutToBadge(records);
            }
        }

        qDebug() << "badges based on position updated to :: " << m_badgesForActivate;
        qDebug() << "badges for applet shortcuts updated to :: " << m_appletShortcuts;

        emit badgesForActivateChanged();
    }
}

void ShortcutsTracker::clearAllAppletShortcuts()
{
    KConfigGroup latteGroup = KConfigGroup(m_shortcutsConfigPtr, "lattedock");

    for(const auto &key : latteGroup.keyList()) {
        if (key.startsWith(APPLETSHORTCUTKEY)) {
            QAction *appletAction = new QAction(this);

            appletAction->setText(QString("Activate ") + key);
            appletAction->setObjectName(key);
            appletAction->setShortcut(QKeySequence());
            KGlobalAccel::setGlobalShortcut(appletAction, QKeySequence());
            KGlobalAccel::self()->removeAllShortcuts(appletAction);

            appletAction->deleteLater();
        }
    }
}


}
}
