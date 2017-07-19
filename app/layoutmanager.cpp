/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
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

#include "layoutmanager.h"

#include <QDir>
#include <QFile>
#include <QtDBus/QtDBus>

#include <KLocalizedString>
#include <KActivities/Consumer>

namespace Latte {

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent),
      m_importer(new Importer(this))
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        //! create the alternative session action
        const QIcon toggleIcon = QIcon::fromTheme("user-identity");
        m_toggleLayoutAction = new QAction(toggleIcon, i18n("Alternative Layout"), this);
        m_toggleLayoutAction->setStatusTip(i18n("Enable/Disable Alternative Layout"));
        m_toggleLayoutAction->setCheckable(true);
        connect(m_toggleLayoutAction, &QAction::triggered, this, &LayoutManager::toggleLayout);

        //! create the add widgets action
        const QIcon addWidIcon = QIcon::fromTheme("add");
        m_addWidgetsAction = new QAction(addWidIcon, i18n("Add Widgets..."), this);
        m_addWidgetsAction->setStatusTip(i18n("Show Plasma Widget Explorer"));
        connect(m_addWidgetsAction, &QAction::triggered, this, &LayoutManager::showWidgetsExplorer);

        connect(m_corona->universalSettings(), &UniversalSettings::currentLayoutNameChanged, this, &LayoutManager::currentLayoutNameChanged);
    }
}

LayoutManager::~LayoutManager()
{
    m_importer->deleteLater();
    m_toggleLayoutAction->deleteLater();

    if (m_currentLayout) {
        m_currentLayout->deleteLater();
    }
}

void LayoutManager::load()
{
    int configVer = m_corona->universalSettings()->version();
    qDebug() << "Universal Settings version : " << configVer;

    if (configVer < 2 && QFile(QDir::homePath() + "/.config/lattedockrc").exists()) {
        qDebug() << "Latte must update its configuration...";
        m_importer->updateOldConfiguration();
    } else if (!QFile(QDir::homePath() + "/.config/lattedockrc").exists()) {
        //startup create what is necessary....
        QDir layoutDir(QDir::homePath() + "/.config/latte");

        if (!layoutDir.exists()) {
            QDir(QDir::homePath() + "/.config").mkdir("latte");
        }

        newLayout(i18n("My Layout"));
        m_corona->universalSettings()->setCurrentLayoutName(i18n("My Layout"));
        m_corona->universalSettings()->setVersion(2);
    }

    qDebug() << "Latte is loading  its layouts...";

    loadLayouts();
}

DockCorona *LayoutManager::corona()
{
    return m_corona;
}

QAction *LayoutManager::toggleLayoutAction()
{
    return m_toggleLayoutAction;
}

QAction *LayoutManager::addWidgetsAction()
{
    return m_addWidgetsAction;
}

LayoutSettings *LayoutManager::currentLayout()
{
    return m_currentLayout;
}

QString LayoutManager::currentLayoutName() const
{
    if (m_corona && m_corona->universalSettings()) {
        return m_corona->universalSettings()->currentLayoutName();
    }

    return QString();
}

QStringList LayoutManager::layouts() const
{
    return m_layouts;
}

QStringList LayoutManager::menuLayouts() const
{
    return m_menuLayouts;
}

QStringList LayoutManager::activities()
{
    return m_corona->m_activityConsumer->activities();
}

QString LayoutManager::layoutPath(QString layoutName)
{
    QString path = QDir::homePath() + "/.config/latte/" + layoutName + ".layout.latte";

    if (!QFile(path).exists()) {
        path = "";
    }

    return path;
}

void LayoutManager::toggleLayout()
{
    if (m_corona->universalSettings()->currentLayoutName() == i18n("Alternative")) {
        switchToLayout(m_lastNonAlternativeLayout);
    } else {
        switchToLayout(i18n("Alternative"));
    }
}

void LayoutManager::loadLayouts()
{
    m_layouts.clear();
    m_menuLayouts.clear();

    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    foreach (auto layout, files) {
        LayoutSettings layoutSets(this, layoutDir.absolutePath() + "/" + layout);

        m_layouts.append(layoutSets.name());

        if (layoutSets.showInMenu()) {
            m_menuLayouts.append(layoutSets.name());
        }
    }

    emit layoutsChanged();
    emit menuLayoutsChanged();
}

bool LayoutManager::switchToLayout(QString layoutName)
{
    if (m_currentLayout && m_currentLayout->name() == layoutName) {
        return false;
    }

    QString lPath = layoutPath(layoutName);

    if (lPath.isEmpty() && layoutName == i18n("Alternative")) {
        lPath = newLayout(i18n("Alternative"), i18n("Default"));
    }

    if (!lPath.isEmpty()) {
        //! this code must be called asynchronously because it is called
        //! also from qml (Tasks plasmoid). This change fixes a very important
        //! crash when switching sessions through the Tasks plasmoid Context menu
        //! Latte was unstable and was crashing very often during changing
        //! sessions
        QTimer::singleShot(0, [this, layoutName, lPath]() {
            qDebug() << layoutName << " - " << lPath;
            m_corona->loadLatteLayout(lPath);
            m_corona->universalSettings()->setCurrentLayoutName(layoutName);

            if (m_currentLayout) {
                m_currentLayout->deleteLater();
            }

            m_currentLayout = new LayoutSettings(this, lPath, layoutName);

            emit currentLayoutChanged();

            if (layoutName != i18n("Alternative")) {
                m_toggleLayoutAction->setChecked(false);
                m_lastNonAlternativeLayout = layoutName;
            } else {
                m_toggleLayoutAction->setChecked(true);
            }
        });
    }
}

QString LayoutManager::newLayout(QString layoutName, QString preset)
{
    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString(layoutName + "*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    //! if the newLayout already exists provide a newName that doesnt
    if (files.count() >= 1) {
        int newCounter = files.count() + 1;

        layoutName = layoutName + "-" + QString::number(newCounter);
    }

    QString newLayoutPath = layoutDir.absolutePath() + "/" + layoutName + ".layout.latte";

    qDebug() << "adding layout : " << layoutName << " based on preset:" << preset;

    if (preset == i18n("Default") && !QFile(newLayoutPath).exists()) {
        qDebug() << "adding layout : succeed";
        QFile(m_corona->kPackage().filePath("preset1")).copy(newLayoutPath);
    }

    return newLayoutPath;
}

void LayoutManager::showLayoutConfigDialog()
{
    if (!m_layoutConfigDialog)
        m_layoutConfigDialog = new LayoutConfigDialog(nullptr, this);

    m_layoutConfigDialog->show();
}

void LayoutManager::showWidgetsExplorer()
{
    QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("toggleWidgetExplorer");
    }
}

}
