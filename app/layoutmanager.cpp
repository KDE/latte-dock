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
#include "infoview.h"


#include <QDir>
#include <QFile>
#include <QQmlProperty>
#include <QtDBus/QtDBus>

#include <KActivities/Consumer>
#include <KLocalizedString>
#include <KNotification>

namespace Latte {

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent),
      m_importer(new Importer(this))
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        //! create the add widgets action
        const QIcon addWidIcon = QIcon::fromTheme("add");
        m_addWidgetsAction = new QAction(addWidIcon, i18n("Add Widgets..."), this);
        m_addWidgetsAction->setStatusTip(i18n("Show Plasma Widget Explorer"));
        connect(m_addWidgetsAction, &QAction::triggered, this, &LayoutManager::showWidgetsExplorer);

        connect(m_corona->universalSettings(), &UniversalSettings::currentLayoutNameChanged, this, &LayoutManager::currentLayoutNameChanged);

        m_dynamicSwitchTimer.setSingleShot(true);
        m_dynamicSwitchTimer.setInterval(2000);
        connect(&m_dynamicSwitchTimer, &QTimer::timeout, this, &LayoutManager::confirmDynamicSwitch);
    }
}

LayoutManager::~LayoutManager()
{
    m_importer->deleteLater();

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
        importPresets(false);
    } else if (!QFile(QDir::homePath() + "/.config/lattedockrc").exists()) {
        //startup create what is necessary....
        QDir layoutDir(QDir::homePath() + "/.config/latte");

        if (!layoutDir.exists()) {
            QDir(QDir::homePath() + "/.config").mkdir("latte");
        }

        newLayout(i18n("My Layout"));
        importPresets(false);
        m_corona->universalSettings()->setCurrentLayoutName(i18n("My Layout"));
        m_corona->universalSettings()->setVersion(2);
    }

    qDebug() << "Latte is loading  its layouts...";

    connect(m_corona->m_activityConsumer, &KActivities::Consumer::currentActivityChanged,
            this, &LayoutManager::currentActivityChanged);

    loadLayouts();
}

DockCorona *LayoutManager::corona()
{
    return m_corona;
}

Importer *LayoutManager::importer()
{
    return m_importer;
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
    QStringList fixedMenuLayouts = m_menuLayouts;

    //! in case the current layout isnt checked to be shown in the menus
    //! we must add it on top
    if (!fixedMenuLayouts.contains(currentLayoutName())) {
        fixedMenuLayouts.prepend(currentLayoutName());
    }

    return fixedMenuLayouts;
}

void LayoutManager::setMenuLayouts(QStringList layouts)
{
    if (m_menuLayouts == layouts) {
        return;
    }

    m_menuLayouts = layouts;
    emit menuLayoutsChanged();
}

QStringList LayoutManager::activities()
{
    return m_corona->m_activityConsumer->activities();
}

QStringList LayoutManager::presetsPaths() const
{
    return m_presetsPaths;
}

QString LayoutManager::layoutPath(QString layoutName)
{
    QString path = QDir::homePath() + "/.config/latte/" + layoutName + ".layout.latte";

    if (!QFile(path).exists()) {
        path = "";
    }

    return path;
}

void LayoutManager::currentActivityChanged(const QString &id)
{
    qDebug() << "activity changed :: " << id;

    m_shouldSwitchToLayout = shouldSwitchToLayout(id);

    m_dynamicSwitchTimer.start();
}

QString LayoutManager::shouldSwitchToLayout(QString activityId)
{
    if (m_assignedLayouts.contains(activityId) && m_assignedLayouts[activityId] != currentLayoutName()) {
        return m_assignedLayouts[activityId];
    } else if (!m_assignedLayouts.contains(activityId) && !m_corona->universalSettings()->lastNonAssignedLayoutName().isEmpty()
               && m_corona->universalSettings()->lastNonAssignedLayoutName() != currentLayoutName()) {
        return m_corona->universalSettings()->lastNonAssignedLayoutName();
    }

    return QString();
}

void LayoutManager::confirmDynamicSwitch()
{
    QString tempShouldSwitch = shouldSwitchToLayout(m_corona->m_activityConsumer->currentActivity());

    if (tempShouldSwitch.isEmpty()) {
        return;
    }

    if (m_shouldSwitchToLayout == tempShouldSwitch && m_shouldSwitchToLayout != currentLayoutName()) {
        qDebug() << "dynamic switch to layout :: " << m_shouldSwitchToLayout;

        if (m_corona->universalSettings()->showInfoWindow()) {
            showInfoWindow(i18n("Switching to layout <b>%0</b> ...").arg(m_shouldSwitchToLayout), 4000);

            QTimer::singleShot(500, [this, tempShouldSwitch]() {
                switchToLayout(tempShouldSwitch);
            });
        } else {
            switchToLayout(m_shouldSwitchToLayout);
        }

    } else {
        m_shouldSwitchToLayout = tempShouldSwitch;
        m_dynamicSwitchTimer.start();
    }
}

void LayoutManager::loadLayouts()
{
    m_layouts.clear();
    m_menuLayouts.clear();
    m_presetsPaths.clear();
    m_assignedLayouts.clear();

    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    foreach (auto layout, files) {
        LayoutSettings layoutSets(this, layoutDir.absolutePath() + "/" + layout);

        QStringList validActivityIds = validActivities(layoutSets.activities());
        layoutSets.setActivities(validActivityIds);

        foreach (auto activity, validActivityIds) {
            m_assignedLayouts[activity] = layoutSets.name();
        }

        m_layouts.append(layoutSets.name());

        if (layoutSets.showInMenu()) {
            m_menuLayouts.append(layoutSets.name());
        }
    }

    m_presetsPaths.append(m_corona->kPackage().filePath("preset1"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset2"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset3"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset4"));

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
        //! sessions.
        QTimer::singleShot(0, [this, layoutName, lPath]() {
            qDebug() << layoutName << " - " << lPath;

            m_corona->loadLatteLayout(lPath);
            m_corona->universalSettings()->setCurrentLayoutName(layoutName);

            if (!layoutIsAssigned(layoutName)) {
                m_corona->universalSettings()->setLastNonAssignedLayoutName(layoutName);
            }

            if (m_currentLayout) {
                m_currentLayout->deleteLater();
            }

            m_currentLayout = new LayoutSettings(this, lPath, layoutName);

            emit currentLayoutChanged();
        });
    }

    return true;
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

void LayoutManager::importPresets(bool includeDefault)
{
    int start = 1;

    if (!includeDefault) {
        start = 2;
    }

    for (int i = start; i <= 4; ++i) {
        QByteArray presetNameOrig = QString("preset" + QString::number(i)).toUtf8();
        QString presetPath = m_corona->kPackage().filePath(presetNameOrig);
        QString presetName = LayoutSettings::layoutName(presetPath);
        QByteArray presetNameChars = presetName.toUtf8();
        presetName = i18n(presetNameChars);

        QString newLayoutFile = QDir::homePath() + "/.config/latte/" + presetName + ".layout.latte";

        if (!QFile(newLayoutFile).exists()) {
            QFile(presetPath).copy(newLayoutFile);
        }
    }
}

QStringList LayoutManager::validActivities(QStringList currentList)
{
    QStringList validIds;

    foreach (auto activity, currentList) {
        if (activities().contains(activity)) {
            validIds.append(activity);
        }
    }

    return validIds;
}

bool LayoutManager::layoutIsAssigned(QString layoutName)
{
    QHashIterator<const QString, QString> i(m_assignedLayouts);

    while (i.hasNext()) {
        i.next();

        if (i.value() == layoutName) {
            return true;
        }
    }

    return false;
}

void LayoutManager::showLayoutConfigDialog()
{
    if (!m_layoutConfigDialog)
        m_layoutConfigDialog = new LayoutConfigDialog(nullptr, this);

    m_layoutConfigDialog->show();
}

void LayoutManager::showInfoWindow(QString info, int duration)
{
    foreach (auto screen, qGuiApp->screens()) {
        InfoView *infoView = new InfoView(m_corona, info, screen);

        infoView->show();

        QTimer::singleShot(duration, [this, infoView]() {
            infoView->deleteLater();
        });
    }
}

void LayoutManager::showWidgetsExplorer()
{
    QDBusInterface iface("org.kde.plasmashell", "/PlasmaShell", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("toggleWidgetExplorer");
    }
}

//! it is used just in order to provide translations for the presets
void LayoutManager::ghostForTranslatedPresets()
{
    QString preset1 = i18n("Default");
    QString preset2 = i18n("Plasma");
    QString preset3 = i18n("Unity");
    QString preset4 = i18n("Extended");
}

}
