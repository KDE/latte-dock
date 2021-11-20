/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "manager.h"

// local
#include "importer.h"
#include "syncedlaunchers.h"
#include "../infoview.h"
#include "../screenpool.h"
#include "../data/layoutdata.h"
#include "../data/generictable.h"
#include "../layout/abstractlayout.h"
#include "../layout/centrallayout.h"
#include "../layouts/storage.h"
#include "../settings/universalsettings.h"
#include "../templates/templatesmanager.h"
#include "../tools/commontools.h"

// Qt
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QLatin1String>

// KDE
#include <KMessageBox>
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
    if (memoryUsage() == Latte::MemoryUsage::MultipleLayouts) {
        m_importer->setMultipleLayoutsStatus(Latte::MultipleLayouts::Paused);
    }

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

    //! Custom Templates path creation
    QDir localTemplatesDir(Latte::configPath() + "/latte/templates");

    if (!localTemplatesDir.exists()) {
        QDir(Latte::configPath() + "/latte").mkdir("templates");
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

QStringList Manager::viewTemplateNames() const
{
    Latte::Data::GenericTable<Data::Generic> viewtemplates = m_corona->templatesManager()->viewTemplates();

    QStringList names;

    for(int i=0; i<viewtemplates.rowCount(); ++i) {
        names << viewtemplates[i].name;
    }

    return names;
}

QStringList Manager::viewTemplateIds() const
{
    Latte::Data::GenericTable<Data::Generic> viewtemplates = m_corona->templatesManager()->viewTemplates();

    QStringList ids;

    for(int i=0; i<viewtemplates.rowCount(); ++i) {
        ids << viewtemplates[i].id;
    }

    return ids;
}

Latte::Data::LayoutIcon Manager::iconForLayout(const QString &storedLayoutName) const
{
    Data::Layout l = m_synchronizer->data(storedLayoutName);
    return iconForLayout(l);
}

Latte::Data::LayoutIcon Manager::iconForLayout(const Data::Layout &layout) const
{
    Latte::Data::LayoutIcon _icon;

    if (!layout.icon.isEmpty()) {
        //! if there is specific icon set from the user for this layout we draw only that icon
        _icon.name = layout.icon;
        _icon.isBackgroundFile = false;
        return _icon;
    }

    //! fallback icon: background image
    if (_icon.isEmpty()) {
        QString colorPath = m_corona->kPackage().path() + "../../shells/org.kde.latte.shell/contents/images/canvas/";

        if (layout.backgroundStyle == Layout::PatternBackgroundStyle && layout.background.isEmpty()) {
            colorPath += "defaultcustomprint.jpg";
        } else {
            colorPath = layout.background.startsWith("/") ? layout.background : colorPath + layout.color + "print.jpg";
        }

        if (QFileInfo(colorPath).exists()) {
            _icon.isBackgroundFile = true;
            _icon.name = colorPath;
            return _icon;
        }
    }

    return Latte::Data::LayoutIcon();
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
 /*   QStringList layouts = m_importer->checkRepairMultipleLayoutsLinkedFile();

    //! Latte didn't close correctly, maybe a crash
    if (layouts.size() > 0) {
        QDialog* dialog = new QDialog(nullptr);
        dialog->setWindowTitle(i18n("Multiple Layouts Startup Warning"));
        dialog->setObjectName("sorry");
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        auto buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok);

        KMessageBox::createKMessageBox(dialog,
                                       buttonbox,
                                       QMessageBox::Warning,
                                       i18np("<b>Multiple Layouts based on Activities</b> mode did not close properly during the last session.<br/><br/>The following layout <b>[ %2 ]</b> had to be updated for consistency!",
                                             "<b>Multiple Layouts based on Activities</b> mode did not close properly during the last session.<br/><br/>The following layouts <b>[ %2 ]</b> had to be updated for consistency!",
                                             layouts.count(),
                                             layouts.join(", ")),
                                       QStringList(),
                                       QString(),
                                       0,
                                       KMessageBox::NoExec,
                                       QString());
        dialog->show();
    }*/

    m_synchronizer->switchToLayout(layoutName);
}

void Manager::moveView(QString originLayoutName, uint originViewId, QString destinationLayoutName)
{
    if (memoryUsage() != Latte::MemoryUsage::MultipleLayouts
            || originLayoutName.isEmpty()
            || destinationLayoutName.isEmpty()
            || originViewId <= 0
            || originLayoutName == destinationLayoutName) {
        return;
    }

    auto originlayout = m_synchronizer->layout(originLayoutName);
    auto destinationlayout = m_synchronizer->layout(destinationLayoutName);

    if (!originlayout || !destinationlayout || originlayout == destinationlayout) {
        return;
    }

    Plasma::Containment *originviewcontainment = originlayout->containmentForId(originViewId);
    Latte::View *originview = originlayout->viewForContainment(originViewId);

    if (!originviewcontainment) {
        return;
    }

    QList<Plasma::Containment *> origincontainments = originlayout->unassignFromLayout(originviewcontainment);

    if (origincontainments.size() > 0) {
        destinationlayout->assignToLayout(originview, origincontainments);
    }
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
    Layouts::Storage::self()->removeAllClonedViews(path);

    KSharedConfigPtr filePtr = KSharedConfig::openConfig(path);

    KConfigGroup actionGroups = KConfigGroup(filePtr, "ActionPlugins");

    QStringList deprecatedActionGroup;

    for (const auto &actId : actionGroups.groupList()) {
        QString pluginId = actionGroups.group(actId).readEntry("RightButton;NoModifier", "");

        if (pluginId == QStringLiteral("org.kde.contextmenu")) {
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

        if (pluginId == QStringLiteral("org.kde.desktopcontainment")) { //!must remove ghost containments first
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
        containment.sync();
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
