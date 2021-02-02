/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*            2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "manager.h"

// local
#include "importer.h"
#include "syncedlaunchers.h"
#include "../infoview.h"
#include "../screenpool.h"
#include "../data/layoutdata.h"
#include "../layout/abstractlayout.h"
#include "../layout/centrallayout.h"
#include "../settings/dialogs/settingsdialog.h"
#include "../settings/universalsettings.h"
#include "../templates/templatesmanager.h"
#include "../tools/commontools.h"

// Qt
#include <QDir>
#include <QFile>
#include <QMessageBox>

// KDE
#include <KLocalizedString>
#include <KNotification>

namespace Latte {
namespace Layouts {

Manager::Manager(QObject *parent)
    : QObject(parent),
      m_importer(new Importer(this)),
      m_syncedLaunchers(new SyncedLaunchers(this))
{
    m_corona = qobject_cast<Latte::Corona *>(parent);
    //! needs to be created AFTER corona assignment
    m_synchronizer = new Synchronizer(this);

    if (m_corona) {

        connect(m_synchronizer, &Synchronizer::centralLayoutsChanged, this, &Manager::centralLayoutsChanged);
        connect(m_synchronizer, &Synchronizer::currentLayoutIsSwitching, this, &Manager::currentLayoutIsSwitching);
    }
}

Manager::~Manager()
{
    m_importer->deleteLater();
    m_syncedLaunchers->deleteLater();

    //! no needed because Latte:Corona is calling it at better place
    // unload();

    m_synchronizer->deleteLater();
}

void Manager::init()
{
    QDir layoutsDir(Layouts::Importer::layoutUserDir());
    bool firstRun = !layoutsDir.exists();

    int configVer = m_corona->universalSettings()->version();
    qDebug() << "Universal Settings version : " << configVer;

    if (firstRun) {
        m_corona->universalSettings()->setVersion(2);
        m_corona->universalSettings()->setSingleModeLayoutName(i18n("My Layout"));

        //startup create what is necessary....
        if (!layoutsDir.exists()) {
            QDir(Latte::configPath()).mkdir("latte");
        }

        QString defpath = m_corona->templatesManager()->newLayout(i18n("My Layout"), i18n(Templates::DEFAULTLAYOUTTEMPLATENAME));
        setOnAllActivities(Layout::AbstractLayout::layoutName(defpath));

        m_corona->templatesManager()->importSystemLayouts();
    } else if (configVer < 2 && !firstRun) {
        m_corona->universalSettings()->setVersion(2);

        bool isOlderVersion = m_importer->updateOldConfiguration();
        if (isOlderVersion) {
            qDebug() << "Latte is updating its older configuration...";
            m_corona->templatesManager()->importSystemLayouts();
        } else {
            m_corona->universalSettings()->setSingleModeLayoutName(i18n("My Layout"));
        }
    }

    //! Check if the multiple-layouts hidden file is present, add it if it isnt
    if (!QFile(Layouts::Importer::layoutUserFilePath(Layout::MULTIPLELAYOUTSHIDDENNAME)).exists()) {
        m_corona->templatesManager()->newLayout("", Layout::MULTIPLELAYOUTSHIDDENNAME);
    }

    qDebug() << "Latte is loading  its layouts...";

    m_synchronizer->initLayouts();
}

void Manager::unload()
{
    m_synchronizer->unloadLayouts();
}

Latte::Corona *Manager::corona()
{
    return m_corona;
}

Importer *Manager::importer()
{
    return m_importer;
}

SyncedLaunchers *Manager::syncedLaunchers() const
{
    return m_syncedLaunchers;
}

Synchronizer *Manager::synchronizer() const
{
    return m_synchronizer;
}

MemoryUsage::LayoutsMemory Manager::memoryUsage() const
{
    return m_corona->universalSettings()->layoutsMemoryUsage();
}

void Manager::setMemoryUsage(MemoryUsage::LayoutsMemory memoryUsage)
{
    m_corona->universalSettings()->setLayoutsMemoryUsage(memoryUsage);
}

QStringList Manager::centralLayoutsNames()
{
    return m_synchronizer->centralLayoutsNames();
}

QStringList Manager::currentLayoutsNames() const
{
    return m_synchronizer->currentLayoutsNames();
}

QList<CentralLayout *> Manager::currentLayouts() const
{
    return m_synchronizer->currentLayouts();
}

bool Manager::switchToLayout(QString layoutName,  MemoryUsage::LayoutsMemory newMemoryUsage)
{
    return m_synchronizer->switchToLayout(layoutName, newMemoryUsage);
}

void Manager::loadLayoutOnStartup(QString layoutName)
{
    QStringList layouts = m_importer->checkRepairMultipleLayoutsLinkedFile();

    //! Latte didn't close correctly, maybe a crash
    if (layouts.size() > 0) {
        QMessageBox *msg = new QMessageBox();
        msg->setAttribute(Qt::WA_DeleteOnClose);
        msg->setIcon(QMessageBox::Warning);
        msg->setWindowTitle(i18n("Multiple Layouts Warning"));
        msg->setText(i18n("Latte did not close properly in the previous session. The following layout(s) <b>[%0]</b> were updated for consistency!!!").arg(layouts.join(",")));
        msg->setStandardButtons(QMessageBox::Ok);

        msg->open();
    }

    m_synchronizer->switchToLayout(layoutName);
}

void Manager::loadLatteLayout(QString layoutPath)
{
    qDebug() << " -------------------------------------------------------------------- ";
    qDebug() << " -------------------------------------------------------------------- ";

    if (m_corona->containments().size() > 0) {
        qDebug() << "LOAD LATTE LAYOUT ::: There are still containments present !!!! :: " << m_corona->containments().size();
    }

    if (!layoutPath.isEmpty() && m_corona->containments().size() == 0) {
        cleanupOnStartup(layoutPath);
        qDebug() << "LOADING CORONA LAYOUT:" << layoutPath;
        m_corona->loadLayout(layoutPath);
    }
}

void Manager::setOnAllActivities(QString layoutName)
{
    CentralLayout *central = m_synchronizer->centralLayout(layoutName);

    if (central) {
        central->setActivities(QStringList(Data::Layout::ALLACTIVITIESID));
    } else if (m_importer->layoutExists(layoutName)) {
        CentralLayout storage(this, m_importer->layoutUserFilePath(layoutName));
        storage.setActivities(QStringList(Data::Layout::ALLACTIVITIESID));
    }
}

void Manager::setOnActivities(QString layoutName, QStringList activities)
{
    CentralLayout *central = m_synchronizer->centralLayout(layoutName);

    if (central) {
        central->setActivities(activities);
    } else if (m_importer->layoutExists(layoutName)) {
        CentralLayout storage(this, m_importer->layoutUserFilePath(layoutName));
        storage.setActivities(activities);
    }
}

void Manager::cleanupOnStartup(QString path)
{
    KSharedConfigPtr filePtr = KSharedConfig::openConfig(path);

    KConfigGroup actionGroups = KConfigGroup(filePtr, "ActionPlugins");

    QStringList deprecatedActionGroup;

    for (const auto &actId : actionGroups.groupList()) {
        QString pluginId = actionGroups.group(actId).readEntry("RightButton;NoModifier", "");

        if (pluginId == "org.kde.contextmenu") {
            deprecatedActionGroup << actId;
        }
    }

    for (const auto &pId : deprecatedActionGroup) {
        qDebug() << "!!!!!!!!!!!!!!!!  !!!!!!!!!!!! !!!!!!! REMOVING :::: " << pId;
        actionGroups.group(pId).deleteGroup();
    }

    KConfigGroup containmentGroups = KConfigGroup(filePtr, "Containments");

    QStringList removeContaimentsList;

    for (const auto &cId : containmentGroups.groupList()) {
        QString pluginId = containmentGroups.group(cId).readEntry("plugin", "");

        if (pluginId == "org.kde.desktopcontainment") { //!must remove ghost containments first
            removeContaimentsList << cId;
        }
    }

    for (const auto &cId : removeContaimentsList) {
        containmentGroups.group(cId).deleteGroup();
    }
}


void Manager::showAboutDialog()
{
    m_corona->aboutApplication();
}

void Manager::clearUnloadedContainmentsFromLinkedFile(QStringList containmentsIds, bool bypassChecks)
{
    if (!m_corona || (memoryUsage() == MemoryUsage::SingleLayout && !bypassChecks)) {
        return;
    }

    auto containments = m_corona->config()->group("Containments");

    for (const auto &conId : containmentsIds) {
        qDebug() << "unloads ::: " << conId;
        KConfigGroup containment = containments.group(conId);
        containment.deleteGroup();
    }

    containments.sync();
}

void Manager::showLatteSettingsDialog(int firstPage, bool toggleCurrentPage)
{
    if (!m_latteSettingsDialog) {
        m_latteSettingsDialog = new Latte::Settings::Dialog::SettingsDialog(nullptr, m_corona);
    }
    m_latteSettingsDialog->show();

    if (m_latteSettingsDialog->isMinimized()) {
        m_latteSettingsDialog->showNormal();
    }

    if (toggleCurrentPage) {
        m_latteSettingsDialog->toggleCurrentPage();
    } else {
        m_latteSettingsDialog->setCurrentPage(firstPage);
    }

    m_latteSettingsDialog->activateWindow();
}

void Manager::hideLatteSettingsDialog()
{
    if (m_latteSettingsDialog) {
        m_latteSettingsDialog->deleteLater();
        m_latteSettingsDialog = nullptr;
    }
}

void Manager::showInfoWindow(QString info, int duration, QStringList activities)
{
    for (const auto screen : qGuiApp->screens()) {
        InfoView *infoView = new InfoView(m_corona, info, screen);

        infoView->show();
        infoView->setOnActivities(activities);

        QTimer::singleShot(duration, [this, infoView]() {
            infoView->deleteLater();
        });
    }
}

}
}
