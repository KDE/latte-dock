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
#include "launcherssignals.h"
#include "../infoview.h"
#include "../screenpool.h"
#include "../layout/abstractlayout.h"
#include "../layout/centrallayout.h"
#include "../settings/settingsdialog.h"
#include "../settings/universalsettings.h"

// Qt
#include <QDir>
#include <QFile>
#include <QMessageBox>

// KDE
#include <KLocalizedString>
#include <KNotification>

namespace Latte {
namespace Layouts {

const int MultipleLayoutsPresetId = 10;

Manager::Manager(QObject *parent)
    : QObject(parent),
      m_importer(new Importer(this)),
      m_launchersSignals(new LaunchersSignals(this))
{
    m_corona = qobject_cast<Latte::Corona *>(parent);
    //! needs to be created AFTER corona assignment
    m_synchronizer = new Synchronizer(this);

    if (m_corona) {
        connect(m_corona->universalSettings(), &UniversalSettings::currentLayoutNameChanged, this, &Manager::currentLayoutNameChanged);

        connect(m_synchronizer, &Synchronizer::centralLayoutsChanged, this, &Manager::centralLayoutsChanged);
        connect(m_synchronizer, &Synchronizer::currentLayoutNameChanged, this, &Manager::currentLayoutNameChanged);
        connect(m_synchronizer, &Synchronizer::currentLayoutIsSwitching, this, &Manager::currentLayoutIsSwitching);
        connect(m_synchronizer, &Synchronizer::layoutsChanged, this, &Manager::layoutsChanged);
        connect(m_synchronizer, &Synchronizer::menuLayoutsChanged, this, &Manager::menuLayoutsChanged);
    }
}

Manager::~Manager()
{
    m_importer->deleteLater();
    m_launchersSignals->deleteLater();

    //! no needed because Latte:Corona is calling it at better place
    // unload();

    m_synchronizer->deleteLater();
}

void Manager::load()
{
    m_presetsPaths.clear();

    QDir layoutsDir(QDir::homePath() + "/.config/latte");
    bool firstRun = !layoutsDir.exists();

    int configVer = m_corona->universalSettings()->version();
    qDebug() << "Universal Settings version : " << configVer;

    if (firstRun) {
        m_corona->universalSettings()->setVersion(2);
        m_corona->universalSettings()->setCurrentLayoutName(i18n("My Layout"));

        //startup create what is necessary....
        if (!layoutsDir.exists()) {
            QDir(QDir::homePath() + "/.config").mkdir("latte");
        }

        newLayout(i18n("My Layout"));
        importPresets(false);
    } else if (configVer < 2 && !firstRun) {
        m_corona->universalSettings()->setVersion(2);

        bool isOlderVersion = m_importer->updateOldConfiguration();
        if (isOlderVersion) {
            qDebug() << "Latte is updating its older configuration...";
            importPresets(false);
        } else {
            m_corona->universalSettings()->setCurrentLayoutName(i18n("My Layout"));
        }
    }

    //! Check if the multiple-layouts hidden file is present, add it if it isnt
    if (!QFile(QDir::homePath() + "/.config/latte/" + Layout::AbstractLayout::MultipleLayoutsName + ".layout.latte").exists()) {
        importPreset(MultipleLayoutsPresetId, false);
    }

    qDebug() << "Latte is loading  its layouts...";

    m_presetsPaths.append(m_corona->kPackage().filePath("preset1"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset2"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset3"));
    m_presetsPaths.append(m_corona->kPackage().filePath("preset4"));

    m_synchronizer->loadLayouts();
}

void Manager::unload()
{
    m_synchronizer->unloadLayouts();

    //! Remove no-needed temp files
    QString temp1File = QDir::homePath() + "/.config/lattedock.copy1.bak";
    QString temp2File = QDir::homePath() + "/.config/lattedock.copy2.bak";

    QFile file1(temp1File);
    QFile file2(temp2File);

    if (file1.exists()) {
        file1.remove();
    }

    if (file2.exists()) {
        file2.remove();
    }
}

Latte::Corona *Manager::corona()
{
    return m_corona;
}

Importer *Manager::importer()
{
    return m_importer;
}

LaunchersSignals *Manager::launchersSignals() const
{
    return m_launchersSignals;
}

Synchronizer *Manager::synchronizer() const
{
    return m_synchronizer;
}

QString Manager::currentLayoutName() const
{
    return m_synchronizer->currentLayoutName();
}

QString Manager::defaultLayoutName() const
{
    QByteArray presetNameOrig = QString("preset" + QString::number(1)).toUtf8();
    QString presetPath = m_corona->kPackage().filePath(presetNameOrig);
    QString presetName = CentralLayout::layoutName(presetPath);
    QByteArray presetNameChars = presetName.toUtf8();
    presetName = i18n(presetNameChars);

    return presetName;
}

QStringList Manager::layouts() const
{
    return m_synchronizer->layouts();
}

QStringList Manager::menuLayouts() const
{
    return m_synchronizer->menuLayouts();
}

void Manager::setMenuLayouts(QStringList layouts)
{
    m_synchronizer->setMenuLayouts(layouts);
}

QStringList Manager::presetsPaths() const
{
    return m_presetsPaths;
}

Types::LayoutsMemoryUsage Manager::memoryUsage() const
{
    return m_corona->universalSettings()->layoutsMemoryUsage();
}

int Manager::layoutsMemoryUsage()
{
    return (int)m_corona->universalSettings()->layoutsMemoryUsage();
}

void Manager::setMemoryUsage(Types::LayoutsMemoryUsage memoryUsage)
{
    m_corona->universalSettings()->setLayoutsMemoryUsage(memoryUsage);
}

QStringList Manager::centralLayoutsNames()
{
    return m_synchronizer->centralLayoutsNames();
}

QStringList Manager::sharedLayoutsNames()
{
    return m_synchronizer->sharedLayoutsNames();
}

QStringList Manager::storedSharedLayouts() const
{
    return m_synchronizer->storedSharedLayouts();
}

CentralLayout *Manager::currentLayout() const
{
    return m_synchronizer->currentLayout();
}

bool Manager::switchToLayout(QString layoutName, int previousMemoryUsage)
{
    return m_synchronizer->switchToLayout(layoutName, previousMemoryUsage);
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


    actionGroups.sync();
    containmentGroups.sync();
}


void Manager::showAboutDialog()
{
    m_corona->aboutApplication();
}

void Manager::clearUnloadedContainmentsFromLinkedFile(QStringList containmentsIds, bool bypassChecks)
{
    if (!m_corona || (memoryUsage() == Types::SingleLayout && !bypassChecks)) {
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

QString Manager::newLayout(QString layoutName, QString preset)
{
    QDir layoutDir(QDir::homePath() + "/.config/latte");
    QStringList filter;
    filter.append(QString(layoutName + "*.layout.latte"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    //! if the newLayout already exists provide a newName that doesn't
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

void Manager::importDefaultLayout(bool newInstanceIfPresent)
{
    importPreset(1, newInstanceIfPresent);

    if (newInstanceIfPresent) {
        m_synchronizer->loadLayouts();
    }
}

void Manager::importPresets(bool includeDefault)
{
    int start = 1;

    if (!includeDefault) {
        start = 2;
    }

    for (int i = start; i <= 4; ++i) {
        importPreset(i, false);
    }
}

void Manager::importPreset(int presetNo, bool newInstanceIfPresent)
{
    QDir configDir(QDir::homePath() + "/.config");

    if (!QDir(configDir.absolutePath() + "/latte").exists()) {
        configDir.mkdir("latte");
    }

    QByteArray presetNameOrig = QString("preset" + QString::number(presetNo)).toUtf8();
    QString presetPath = m_corona->kPackage().filePath(presetNameOrig);
    QString presetName = Layout::AbstractLayout::layoutName(presetPath);
    QByteArray presetNameChars = presetName.toUtf8();
    presetName = i18n(presetNameChars);

    //! hide the multiple layouts layout file from user
    presetName = (presetNo == MultipleLayoutsPresetId) ? "." + presetName : presetName;

    QString newLayoutFile = "";

    if (newInstanceIfPresent) {
        newLayoutFile = QDir::homePath() + "/.config/latte/" + m_importer->uniqueLayoutName(presetName) + ".layout.latte";
    } else {
        newLayoutFile = QDir::homePath() + "/.config/latte/" + presetName + ".layout.latte";
    }

    if (!QFile(newLayoutFile).exists()) {
        QFile(presetPath).copy(newLayoutFile);
        QFileInfo newFileInfo(newLayoutFile);

        if (newFileInfo.exists() && !newFileInfo.isWritable()) {
            QFile(newLayoutFile).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        }
    }
}

void Manager::showLatteSettingsDialog(int page)
{
    bool created{false};

    if (!m_latteSettingsDialog) {
        m_latteSettingsDialog = new SettingsDialog(nullptr, m_corona);
        created = true;
    }
    m_latteSettingsDialog->show();

    if (m_latteSettingsDialog->isMinimized()) {
        m_latteSettingsDialog->showNormal();
    }

    if (!created) {
        Types::LatteConfigPage configPage = static_cast<Types::LatteConfigPage>(page);
        m_latteSettingsDialog->toggleCurrentPage();
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

//! it is used just in order to provide translations for the presets
void Manager::ghostForTranslatedPresets()
{
    QString preset1 = i18n("Default");
    QString preset2 = i18n("Plasma");
    QString preset3 = i18n("Unity");
    QString preset4 = i18n("Extended");
}

}
}
