/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

// local
#include "shortcutstracker.h"

// Qt
#include <QAction>
#include <QDir>
#include <QDebug>

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

    const QString globalShortcutsFilePath = QDir::homePath() + "/.config/" + GLOBALSHORTCUTSCONFIG;
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

QList<int> ShortcutsTracker::appletsWithPlasmaShortcuts()
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
            if (modifiers.count() == 2 && modifiers[0] == "Meta") {
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

    //! make sure that latte dock records in global shortcuts where found correctly
    bool recordExists{true};

    if (!latteGroup.exists()) {
        recordExists = false;
    }

    if (recordExists) {
        for (int i = 1; i <= 19; ++i) {
            QString entry = "activate entry " + QString::number(i);

            if (!latteGroup.hasKey(entry)) {
                recordExists = false;
                break;
            }
        }
    }

    if (recordExists) {
        m_badgesForActivate.clear();
        m_appletShortcuts.clear();

        for (int i = 1; i <= 19; ++i) {
            QString entry = "activate entry " + QString::number(i);
            QStringList records = latteGroup.readEntry(entry, QStringList());

            m_badgesForActivate << shortcutToBadge(records);
        }

        m_basedOnPositionEnabled = (!m_badgesForActivate[0].isEmpty() && !m_badgesForActivate[1].isEmpty());

        foreach(auto key, latteGroup.keyList()) {
            if (key.startsWith(APPLETSHORTCUTKEY)) {
                QStringList records = latteGroup.readEntry(key, QStringList());
                int appletId = key.remove(APPLETSHORTCUTKEY).toInt();

                m_appletShortcuts[appletId] = shortcutToBadge(records);
            }
        }

        qDebug() << "badges updated to :: " << m_badgesForActivate;
        qDebug() << "applet shortcuts updated to :: " << m_appletShortcuts;

        emit badgesForActivateChanged();
    }
}

void ShortcutsTracker::clearAllAppletShortcuts()
{
    KConfigGroup latteGroup = KConfigGroup(m_shortcutsConfigPtr, "lattedock");

    foreach(auto key, latteGroup.keyList()) {
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
