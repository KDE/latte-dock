/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tablayoutshandler.h"

//! local
#include <coretypes.h>
#include "ui_settingsdialog.h"
#include "settingsdialog.h"
#include "layoutscontroller.h"
#include "layoutsmodel.h"
#include "layoutstableview.h"
#include "../universalsettings.h"
#include "../detailsdialog/detailsdialog.h"
#include "../exporttemplatedialog/exporttemplatedialog.h"
#include "../viewsdialog/viewsdialog.h"
#include "../../apptypes.h"
#include "../../lattecorona.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/importer.h"
#include "../../layouts/manager.h"
#include "../../layouts/storage.h"
#include "../../templates/templatesmanager.h"
#include "../../tools/commontools.h"

//! Qt
#include <QDBusInterface>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>

//! KDE
#include <KWindowSystem>
#include <KLocalizedString>
#include <KActivities/Controller>
#include <KIO/OpenFileManagerWindowJob>
#include <KNewStuff3/KNS3/DownloadDialog>


namespace Latte {
namespace Settings {
namespace Handler {

TabLayouts::TabLayouts(Settings::Dialog::SettingsDialog *parent)
    : Generic(parent),
      m_parentDialog(parent),
      m_corona(m_parentDialog->corona()),
      m_ui(m_parentDialog->ui()),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog").group("TabLayouts"))
{
    //! load first the layouts view column widths
    loadConfig();
    m_layoutsController = new Settings::Controller::Layouts(this);

    //! create menu and assign actions before initializing the user interface
    initLayoutMenu();
    initUi();
}

TabLayouts::~TabLayouts()
{
    saveConfig();
}

void TabLayouts::initUi()
{
    m_inMemoryButtons = new QButtonGroup(this);
    m_inMemoryButtons->addButton(m_ui->singleToolBtn, MemoryUsage::SingleLayout);
    m_inMemoryButtons->addButton(m_ui->multipleToolBtn, MemoryUsage::MultipleLayouts);
    m_inMemoryButtons->setExclusive(true);

    bool inMultiple{m_corona->layoutsManager()->memoryUsage() == MemoryUsage::MultipleLayouts};

    if (inMultiple) {
        m_ui->multipleToolBtn->setChecked(true);
    } else {
        m_ui->singleToolBtn->setChecked(true);
    }

    connect(m_ui->layoutsView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Generic::dataChanged);

    connect(m_layoutsController, &Settings::Controller::Layouts::dataChanged, this, &Generic::dataChanged);

    connect(this, &Settings::Handler::TabLayouts::dataChanged, this, &TabLayouts::updatePerLayoutButtonsState);
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged, this, &TabLayouts::updatePerLayoutButtonsState);

    connect(m_inMemoryButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {

        if (checked) {
            m_layoutsController->setInMultipleMode(id == MemoryUsage::MultipleLayouts);

            if (id == MemoryUsage::MultipleLayouts) {
                m_layoutsController->sortByColumn(Model::Layouts::ACTIVITYCOLUMN, Qt::AscendingOrder);
            } else {
                m_layoutsController->sortByColumn(Model::Layouts::NAMECOLUMN, Qt::AscendingOrder);
            }
        }
    });

    connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &TabLayouts::currentPageChanged);
    connect(this, &TabLayouts::currentPageChanged, this, &TabLayouts::onCurrentPageChanged);

    updatePerLayoutButtonsState();
}

void TabLayouts::initLayoutMenu()
{
    if (!m_layoutMenu) {
        m_layoutMenu = new QMenu(i18n("Layout"), m_parentDialog->appMenuBar());
        m_parentDialog->appMenuBar()->insertMenu(m_parentDialog->helpMenu()->menuAction(), m_layoutMenu);
    }

    m_switchLayoutAction = m_layoutMenu->addAction(i18nc("switch layout","Switch"));
    m_switchLayoutAction->setToolTip(i18n("Switch to selected layout"));
    m_switchLayoutAction->setIcon(QIcon::fromTheme("user-identity"));
    m_switchLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Tab));
    connectActionWithButton(m_ui->switchButton, m_switchLayoutAction);
    connect(m_switchLayoutAction, &QAction::triggered, this, &TabLayouts::switchLayout);

    m_activitiesManagerAction = m_layoutMenu->addAction(i18n("&Activities"));
    m_activitiesManagerAction->setToolTip(i18n("Show Plasma Activities manager"));
    m_activitiesManagerAction->setIcon(QIcon::fromTheme("activities"));
    m_activitiesManagerAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
    connectActionWithButton(m_ui->activitiesButton, m_activitiesManagerAction);
    connect(m_activitiesManagerAction, &QAction::triggered, this, &TabLayouts::toggleActivitiesManager);

    m_layoutMenu->addSeparator();

    m_newLayoutAction = m_layoutMenu->addAction(i18nc("new layout", "&New"));
    m_newLayoutAction->setToolTip(i18n("New layout"));
    m_newLayoutAction->setIcon(QIcon::fromTheme("add"));
    m_newLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    connectActionWithButton(m_ui->newButton, m_newLayoutAction);
    connect(m_newLayoutAction, &QAction::triggered, m_ui->newButton, &QPushButton::showMenu);

    initLayoutTemplatesSubMenu();
    m_newLayoutAction->setMenu(m_layoutTemplatesSubMenu);
    m_ui->newButton->setMenu(m_layoutTemplatesSubMenu);

    connect(m_corona->templatesManager(), &Latte::Templates::Manager::layoutTemplatesChanged, this, &TabLayouts::initLayoutTemplatesSubMenu);

    m_duplicateLayoutAction = m_layoutMenu->addAction(i18nc("duplicate layout", "&Duplicate"));
    m_duplicateLayoutAction->setToolTip(i18n("Duplicate selected layout"));
    m_duplicateLayoutAction->setIcon(QIcon::fromTheme("edit-copy"));
    m_duplicateLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    connectActionWithButton(m_ui->duplicateButton, m_duplicateLayoutAction);
    connect(m_duplicateLayoutAction, &QAction::triggered, this, &TabLayouts::duplicateLayout);

    m_removeLayoutAction = m_layoutMenu->addAction(i18nc("remove layout", "Remove"));
    m_removeLayoutAction->setToolTip(i18n("Remove selected layout"));
    m_removeLayoutAction->setIcon(QIcon::fromTheme("delete"));
    m_removeLayoutAction->setShortcut(QKeySequence(Qt::Key_Delete));
    connectActionWithButton(m_ui->removeButton, m_removeLayoutAction);
    connect(m_removeLayoutAction, &QAction::triggered, this, &TabLayouts::removeLayout);
    m_ui->removeButton->addAction(m_removeLayoutAction); //this is needed in order to be triggered properly

    m_layoutMenu->addSeparator();

    m_enabledLayoutAction = m_layoutMenu->addAction(i18n("Ena&bled"));
    m_enabledLayoutAction->setToolTip(i18n("Assign in activities in order to be activated through Plasma Activities"));
    m_enabledLayoutAction->setIcon(QIcon::fromTheme("edit-link"));
    m_enabledLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
    m_enabledLayoutAction->setCheckable(true);
    connectActionWithButton(m_ui->enabledButton, m_enabledLayoutAction);
    connect(m_enabledLayoutAction, &QAction::triggered, this, &TabLayouts::toggleEnabledLayout);

    m_readOnlyLayoutAction = m_layoutMenu->addAction(i18nc("read only layout", "&Read Only"));
    m_readOnlyLayoutAction->setToolTip(i18n("Make selected layout read-only"));
    m_readOnlyLayoutAction->setIcon(QIcon::fromTheme("object-locked"));
    m_readOnlyLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    m_readOnlyLayoutAction->setCheckable(true);
    connectActionWithButton(m_ui->readOnlyButton, m_readOnlyLayoutAction);
    connect(m_readOnlyLayoutAction, &QAction::triggered, this, &TabLayouts::lockLayout);

    m_viewsAction = m_layoutMenu->addAction(i18nc("layout docks / panels", "Docks, &Panels..."));
    m_viewsAction->setToolTip(i18n("Show selected layouts docks and panels"));
    m_viewsAction->setIcon(QIcon::fromTheme("window"));
    m_viewsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connectActionWithButton(m_ui->viewsBtn, m_viewsAction);
    connect(m_viewsAction, &QAction::triggered, this, &TabLayouts::showViewsDialog);

    m_detailsAction = m_layoutMenu->addAction(i18nc("layout details", "De&tails..."));
    m_detailsAction->setToolTip(i18n("Show selected layout details"));
    m_detailsAction->setIcon(QIcon::fromTheme("view-list-details"));
    m_detailsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    connectActionWithButton(m_ui->detailsButton, m_detailsAction);
    connect(m_detailsAction, &QAction::triggered, this, &TabLayouts::showDetailsDialog);

    m_layoutMenu->addSeparator();


    //! Import
    m_importLayoutAction = m_layoutMenu->addAction(i18nc("import layout", "&Import"));
    m_importLayoutAction->setToolTip(i18n("Import layout from various resources"));
    m_importLayoutAction->setIcon(QIcon::fromTheme("document-import"));
    m_importLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    connectActionWithButton(m_ui->importButton, m_importLayoutAction);
    connect(m_importLayoutAction, &QAction::triggered, m_ui->importButton, &QPushButton::showMenu);

    initImportLayoutSubMenu();
    m_importLayoutAction->setMenu(m_layoutImportSubMenu);
    m_ui->importButton->setMenu(m_layoutImportSubMenu);

    //! Export
    m_exportLayoutAction = m_layoutMenu->addAction(i18nc("export layout", "&Export"));
    m_exportLayoutAction->setToolTip(i18n("Export selected layout at your system"));
    m_exportLayoutAction->setIcon(QIcon::fromTheme("document-export"));
    m_exportLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    connectActionWithButton(m_ui->exportButton, m_exportLayoutAction);
    connect(m_exportLayoutAction, &QAction::triggered, m_ui->exportButton, &QPushButton::showMenu);

    initExportLayoutSubMenu();
    m_exportLayoutAction->setMenu(m_layoutExportSubMenu);
    m_ui->exportButton->setMenu(m_layoutExportSubMenu);
}

void TabLayouts::initImportLayoutSubMenu()
{
    if (!m_layoutImportSubMenu) {
        m_layoutImportSubMenu = new QMenu(m_layoutMenu);
        m_layoutImportSubMenu->setMinimumWidth(m_ui->importButton->width() * 2);
    } else {
        m_layoutImportSubMenu->clear();
    }

    QAction *importLayoutAction = m_layoutImportSubMenu->addAction(i18nc("import layout", "&Import From Local File..."));
    importLayoutAction->setIcon(QIcon::fromTheme("document-import"));
    importLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    connect(importLayoutAction, &QAction::triggered, this, &TabLayouts::importLayout);

    QAction *downloadLayoutAction = m_layoutImportSubMenu->addAction(i18nc("download layout", "Import From K&DE Online Store..."));
    downloadLayoutAction->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
    downloadLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connect(downloadLayoutAction, &QAction::triggered, this, &TabLayouts::downloadLayout);
}

void TabLayouts::initExportLayoutSubMenu()
{
    if (!m_layoutExportSubMenu) {
        m_layoutExportSubMenu = new QMenu(m_layoutMenu);
        m_layoutExportSubMenu->setMinimumWidth(m_ui->exportButton->width() * 2);
    } else {
        m_layoutExportSubMenu->clear();
    }

    QAction *exportForBackup = m_layoutExportSubMenu->addAction(i18nc("export for backup","&Export For Backup..."));
    exportForBackup->setIcon(QIcon::fromTheme("document-export"));
    exportForBackup->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_E));
    connect(exportForBackup, &QAction::triggered, this, &TabLayouts::exportLayoutForBackup);

    QAction *exportAsTemplate = m_layoutExportSubMenu->addAction(i18nc("export as template","Export As &Template..."));
    exportAsTemplate->setIcon(QIcon::fromTheme("document-export"));
    exportAsTemplate->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_T));
    connect(exportAsTemplate, &QAction::triggered, this, &TabLayouts::exportLayoutAsTemplate);
}

void TabLayouts::initLayoutTemplatesSubMenu()
{
    if (!m_layoutTemplatesSubMenu) {
        m_layoutTemplatesSubMenu = new QMenu(m_layoutMenu);
        m_layoutTemplatesSubMenu->setMinimumWidth(m_ui->newButton->width() * 2);
    } else {
        m_layoutTemplatesSubMenu->clear();
    }

    /*Add Layout Templates for New Action*/
    Data::LayoutsTable templates = m_corona->templatesManager()->layoutTemplates();

    bool customtemplateseparatoradded{false};

    for (int i=0; i<templates.rowCount(); ++i) {
        if (!customtemplateseparatoradded && !templates[i].isSystemTemplate()) {
            m_layoutTemplatesSubMenu->addSeparator();
            customtemplateseparatoradded = true;
        }

        QAction *newlayout = m_layoutTemplatesSubMenu->addAction(templates[i].name);
        newlayout->setIcon(QIcon::fromTheme("document-new"));
        QString templatename = templates[i].name;

        connect(newlayout, &QAction::triggered, this, [&, templatename]() {
            newLayout(templatename);
        });
    }

    if (templates.rowCount() > 0) {
        QAction *openTemplatesDirectory = m_layoutTemplatesSubMenu->addAction(i18n("Templates..."));
        openTemplatesDirectory->setToolTip(i18n("Open templates directory"));
        openTemplatesDirectory->setIcon(QIcon::fromTheme("edit"));

        connect(openTemplatesDirectory, &QAction::triggered, this, [&]() {
            KIO::highlightInFileManager({QString(Latte::configPath() + "/latte/templates/Dock.layout.latte")});
        });
    }
}

Latte::Corona *TabLayouts::corona() const
{
    return m_corona;
}

Settings::Dialog::SettingsDialog *TabLayouts::dialog() const
{
    return m_parentDialog;
}

Ui::SettingsDialog *TabLayouts::ui() const
{
    return m_ui;
}

Controller::Layouts *TabLayouts::layoutsController() const
{
    return m_layoutsController;
}

bool TabLayouts::hasChangedData() const
{
    return m_layoutsController->hasChangedData();
}

bool TabLayouts::inDefaultValues() const
{
    return true;
}

bool TabLayouts::isViewsDialogVisible() const
{
    return m_isViewsDialogVisible;
}

void TabLayouts::reset()
{
    m_layoutsController->reset();

    if (m_layoutsController->inMultipleMode()) {
        m_ui->multipleToolBtn->setChecked(true);
    } else {
        m_ui->singleToolBtn->setChecked(true);
    }
}

void TabLayouts::resetDefaults()
{
    //do nothing because there are no defaults
}

void TabLayouts::save()
{
    m_layoutsController->save();
}

void TabLayouts::switchLayout()
{
    if (!isCurrentTab() || !m_switchLayoutAction->isEnabled()) {
        return;
    }

    Latte::Data::Layout selectedLayoutCurrent = m_layoutsController->selectedLayoutCurrentData();
    Latte::Data::Layout selectedLayoutOriginal = m_layoutsController->selectedLayoutOriginalData();
    selectedLayoutOriginal = selectedLayoutOriginal.isEmpty() ? selectedLayoutCurrent : selectedLayoutOriginal;

    if (m_layoutsController->layoutsAreChanged()) {
        showInlineMessage(i18nc("settings:not permitted switching layout","You need to <b>apply</b> your changes first to switch layout afterwards..."),
                          KMessageWidget::Warning);
        return;
    }

    m_layoutsController->setOriginalInMultipleMode(false);
    m_corona->layoutsManager()->switchToLayout(selectedLayoutOriginal.name, MemoryUsage::SingleLayout);

    updatePerLayoutButtonsState();
}

void TabLayouts::toggleActivitiesManager()
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                          QStringLiteral("/PlasmaShell"),
                                                          QStringLiteral("org.kde.PlasmaShell"),
                                                          QStringLiteral("toggleActivityManager"));

    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

void TabLayouts::toggleEnabledLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_layoutsController->inMultipleMode()) {
        return;
    }

    m_layoutsController->toggleEnabledForSelected();

    updatePerLayoutButtonsState();
}

void TabLayouts::updatePerLayoutButtonsState()
{
    //! UI Elements that need to be shown/hidden
    setTwinProperty(m_switchLayoutAction, TWINVISIBLE, !m_layoutsController->inMultipleMode());
    setTwinProperty(m_activitiesManagerAction, TWINVISIBLE, m_layoutsController->inMultipleMode());
    setTwinProperty(m_enabledLayoutAction, TWINVISIBLE, m_layoutsController->inMultipleMode());

    if (!m_layoutsController->hasSelectedLayout()) {
        setTwinProperty(m_enabledLayoutAction, TWINENABLED, false);
        return;
    }

    Latte::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    //! Switch Button
    setTwinProperty(m_switchLayoutAction, TWINENABLED, true);

    //! Enabled Button
    setTwinProperty(m_enabledLayoutAction, TWINENABLED, true);
    setTwinProperty(m_enabledLayoutAction, TWINCHECKED, !selectedLayout.activities.isEmpty());

    //! Layout Read-Only Button
    setTwinProperty(m_readOnlyLayoutAction, TWINCHECKED, selectedLayout.isLocked);

    //! Details Button
    setTwinProperty(m_detailsAction, TWINENABLED, true);
}

void TabLayouts::newLayout(const QString &templateName)
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_newLayoutAction->isEnabled()) {
        return;
    }

    //! retrieve Default layout template
    Data::Layout tdata = m_corona->templatesManager()->layoutTemplateForName(templateName);

    if (!tdata.isNull()) {
        Data::Layout newlayout = m_layoutsController->addLayoutForFile(tdata.id, tdata.name, true);

        if (newlayout.errors == 0 && newlayout.warnings == 0) {
            showInlineMessage(i18nc("settings:layout added successfully","Layout <b>%1</b> added successfully...", newlayout.name),
                              KMessageWidget::Positive);
        }
    }
}

void TabLayouts::duplicateLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_duplicateLayoutAction->isEnabled()) {
        return;
    }

    m_layoutsController->duplicateSelectedLayout();
}

void TabLayouts::downloadLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab()) {
        return;
    }

    KNS3::DownloadDialog dialog(QStringLiteral("latte-layouts.knsrc"), m_parentDialog);
    dialog.resize(m_parentDialog->downloadWindowSize());
    dialog.exec();

    if (!dialog.changedEntries().isEmpty() && !dialog.installedEntries().isEmpty()) {
        for (const auto &entry : dialog.installedEntries()) {
            for (const auto &entryFile : entry.installedFiles()) {
                Latte::Layouts::Importer::LatteFileVersion version = Latte::Layouts::Importer::fileVersion(entryFile);

                if (version == Latte::Layouts::Importer::LayoutVersion2) {
                    Latte::Data::Layout downloaded = m_layoutsController->addLayoutForFile(entryFile);
                    showInlineMessage(i18nc("settings:layout downloaded successfully","Layout <b>%1</b> downloaded successfully...", downloaded.name),
                                      KMessageWidget::Positive);
                    break;
                }
            }
        }
    }

    m_parentDialog->setDownloadWindowSize(dialog.size());
}

void TabLayouts::removeLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_removeLayoutAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    if (selectedLayout.isActive) {
        showInlineMessage(i18nc("settings: active layout remove","<b>Active</b> layouts can not be removed..."),
                          KMessageWidget::Error);
        return;
    }

    if (selectedLayout.isLocked) {
        showInlineMessage(i18nc("settings: locked layout remove","Locked layouts can not be removed..."),
                          KMessageWidget::Error);
        return;
    }

    m_layoutsController->removeSelected();
}

void TabLayouts::lockLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_readOnlyLayoutAction->isEnabled()) {
        return;
    }

    m_layoutsController->toggleLockedForSelected();

    updatePerLayoutButtonsState();
}

void TabLayouts::importLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_importLayoutAction->isEnabled()) {
        return;
    }

    QFileDialog *importFileDialog = new QFileDialog(m_parentDialog, i18nc("import layout", "Import Layout"), QDir::homePath(), QStringLiteral("layout.latte"));

    importFileDialog->setWindowIcon(QIcon::fromTheme("document-import"));
    importFileDialog->setLabelText(QFileDialog::Accept, i18nc("import layout","Import"));
    importFileDialog->setFileMode(QFileDialog::AnyFile);
    importFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    importFileDialog->setDefaultSuffix("layout.latte");

    QStringList filters;
    filters << QString(i18nc("import latte layout", "Latte Dock Layout file v0.2") + "(*.layout.latte)")
            << QString(i18nc("import older latte layout", "Latte Dock Layout file v0.1") + "(*.latterc)");
    importFileDialog->setNameFilters(filters);

    connect(importFileDialog, &QFileDialog::finished, importFileDialog, &QFileDialog::deleteLater);

    connect(importFileDialog, &QFileDialog::fileSelected, this, [&](const QString & file) {
        Latte::Layouts::Importer::LatteFileVersion version = Latte::Layouts::Importer::fileVersion(file);
        qDebug() << "VERSION :::: " << version;

        if (version == Latte::Layouts::Importer::LayoutVersion2) {
            Latte::Data::Layout importedlayout = m_layoutsController->addLayoutForFile(file);
            showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%1</b> imported successfully...", importedlayout.name),
                              KMessageWidget::Positive);
        } else if (version == Latte::Layouts::Importer::ConfigVersion1) {
            if (!m_layoutsController->importLayoutsFromV1ConfigFile(file)) {
                showInlineMessage(i18nc("settings:deprecated layouts import failed","Import layouts from deprecated version <b>failed</b>..."),
                                  KMessageWidget::Error,
                                  true);
            }
        }
    });

    importFileDialog->open();
}

void TabLayouts::exportLayoutAsTemplate()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_exportLayoutAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Data::Layout o_layout = m_layoutsController->selectedLayoutOriginalData();
    Data::Layout c_layout = m_layoutsController->selectedLayoutCurrentData();

    Data::Layout exp_layout = o_layout;
    exp_layout.name = c_layout.name;

    CentralLayout *central =  m_layoutsController->centralLayout(c_layout.id);

    if (central->isActive()) {
        central->syncToLayoutFile();
    }

    Dialog::ExportTemplateDialog *exportDlg = new Dialog::ExportTemplateDialog(m_parentDialog, exp_layout);
    exportDlg->exec();
}

void TabLayouts::exportLayoutForBackup()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_exportLayoutAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    //! Update ALL active original layouts before exporting,
    m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();
    m_corona->universalSettings()->syncSettings();

    QFileDialog *exportFileDialog = new QFileDialog(m_parentDialog, i18n("Export Layout For Backup"), QDir::homePath(), QStringLiteral("layout.latte"));

    exportFileDialog->setLabelText(QFileDialog::Accept, i18nc("export layout","Export"));
    exportFileDialog->setFileMode(QFileDialog::AnyFile);
    exportFileDialog->setAcceptMode(QFileDialog::AcceptSave);
    exportFileDialog->setDefaultSuffix("layout.latte");

    QStringList filters;
    QString filter1(i18nc("export layout", "Latte Dock Layout file v0.2") + "(*.layout.latte)");

    filters << filter1;

    exportFileDialog->setNameFilters(filters);

    connect(exportFileDialog, &QFileDialog::finished, exportFileDialog, &QFileDialog::deleteLater);

    connect(exportFileDialog, &QFileDialog::fileSelected, this, [ &, selectedLayout](const QString & file) {
        auto showExportLayoutError = [this](const Latte::Data::Layout &layout) {
            showInlineMessage(i18nc("settings:layout export fail","Layout <b>%1</b> export <b>failed</b>...", layout.name),
                              KMessageWidget::Error,
                              true);
        };

        if (QFile::exists(file) && !QFile::remove(file)) {
            showExportLayoutError(selectedLayout);
            return;
        }

        if (file.endsWith(".layout.latte")) {
            if (!QFile(selectedLayout.id).copy(file)) {
                showExportLayoutError(selectedLayout);
                return;
            }

            QFileInfo newFileInfo(file);

            if (newFileInfo.exists() && !newFileInfo.isWritable()) {
                QFile(file).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            }

            // cleanup clones from exported file
            Latte::Layouts::Storage::self()->removeAllClonedViews(file);

            CentralLayout layoutS(this, file);
            layoutS.setActivities(QStringList());
            layoutS.clearLastUsedActivity();

            QAction *openUrlAction = new QAction(i18n("Open Location..."), this);
            openUrlAction->setData(file);
            QList<QAction *> actions;
            actions << openUrlAction;

            connect(openUrlAction, &QAction::triggered, this, [&, openUrlAction]() {
                QString file = openUrlAction->data().toString();

                if (!file.isEmpty()) {
                    KIO::highlightInFileManager({file});
                }
            });

            showInlineMessage(i18nc("settings:layout export success","Layout <b>%1</b> export succeeded...", selectedLayout.name),
                              KMessageWidget::Positive,
                              false,
                              actions);
        } else if (file.endsWith(".latterc")) {
            auto showExportConfigurationError = [this]() {
                showInlineMessage(i18n("Full configuration export <b>failed</b>..."),
                                  KMessageWidget::Error,
                                  true);
            };

            if (m_corona->layoutsManager()->importer()->exportFullConfiguration(file)) {
                QAction *openUrlAction = new QAction(i18n("Open Location..."), this);
                openUrlAction->setIcon(QIcon::fromTheme("document-open"));
                openUrlAction->setData(file);
                QList<QAction *> actions;
                actions << openUrlAction;

                connect(openUrlAction, &QAction::triggered, this, [&, openUrlAction]() {
                    QString file = openUrlAction->data().toString();

                    if (!file.isEmpty()) {
                        KIO::highlightInFileManager({file});
                    }
                });

                showInlineMessage(i18n("Full configuration export succeeded..."),
                                  KMessageWidget::Positive,
                                  false,
                                  actions);
            } else {
                showExportConfigurationError();
            }
        }
    });

    exportFileDialog->open();
    exportFileDialog->selectFile(selectedLayout.name + ".layout.latte");
}

void TabLayouts::showDetailsDialog()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_detailsAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();
    auto detailsDlg = new Settings::Dialog::DetailsDialog(m_parentDialog, m_layoutsController);

    detailsDlg->exec();
}

void TabLayouts::showViewsDialog()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_viewsAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();
    auto viewsDlg = new Settings::Dialog::ViewsDialog(m_parentDialog, m_layoutsController);

    m_isViewsDialogVisible = true;
    viewsDlg->exec();
    m_isViewsDialogVisible = false;
}

void TabLayouts::onLayoutFilesDropped(const QStringList &paths)
{
    QStringList layoutNames;

    for (int i=0; i<paths.count(); ++i) {
        if (paths[i].endsWith(".layout.latte")) {
            Latte::Data::Layout importedlayout = m_layoutsController->addLayoutForFile(paths[i]);
            layoutNames << importedlayout.name;
        }
    }

    if(layoutNames.count() > 0) {
        showInlineMessage(i18ncp("settings:layout imported successfully",
                                 "Layout <b>%2</b> imported successfully...",
                                 "Layouts <b>%2</b> imported successfully...",
                                 layoutNames.count(),
                                 layoutNames.join(", ")),
                KMessageWidget::Positive);
    }
}

void TabLayouts::onRawLayoutDropped(const QString &rawLayout)
{
    Latte::Data::Layout importedlayout = m_layoutsController->addLayoutByText(rawLayout);
    showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%1</b> imported successfully...", importedlayout.name),
                      KMessageWidget::Positive);
}

bool TabLayouts::isCurrentTab() const
{
    return (m_parentDialog->currentPage() == Dialog::LayoutPage);
}

bool TabLayouts::isHoveringLayoutsTable(const QPoint &pos)
{
    QPoint topLeft = m_ui->layoutsView->mapTo(m_parentDialog, QPoint(0,0));
    QRect geometry = m_ui->layoutsView->rect();
    geometry.moveTopLeft(topLeft);

    return geometry.contains(pos);
}


void TabLayouts::onCurrentPageChanged()
{
    //int page = m_dialog->currentPage();
    Dialog::ConfigurationPage cPage= m_parentDialog->currentPage();// static_cast<Dialog::ConfigurationPage>(page);

    if (cPage == Dialog::LayoutPage) {
        m_layoutMenu->setEnabled(true);
        m_layoutMenu->menuAction()->setVisible(true);

    } else {
        m_layoutMenu->menuAction()->setVisible(false);
        m_layoutMenu->setEnabled(false);
    }
}

void TabLayouts::onDragEnterEvent(QDragEnterEvent *event)
{
    if (!isHoveringLayoutsTable(event->pos())) {
        return;
    }

    event->acceptProposedAction();
    m_ui->layoutsView->dragEntered(event);
}

void TabLayouts::onDragLeaveEvent(QDragLeaveEvent *event)
{
    m_ui->layoutsView->dragLeft();
}

void TabLayouts::onDragMoveEvent(QDragMoveEvent *event)
{
    if (!isHoveringLayoutsTable(event->pos())) {
        event->ignore();
        m_ui->layoutsView->dragLeft();
        return;
    }

    event->acceptProposedAction();
}

void TabLayouts::onDropEvent(QDropEvent *event)
{
    if (!isHoveringLayoutsTable(event->pos())) {
        event->ignore();
        m_ui->layoutsView->dragLeft();
        return;
    }

    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urlList = event->mimeData()->urls();

        QStringList paths;

        for (int i = 0; i < qMin(urlList.size(), 20); ++i) {
            QString layoutPath = urlList[i].path();

            if (layoutPath.endsWith(".layout.latte")) {
                paths << layoutPath;
            }
        }

        if (paths.count() > 0) {
            onLayoutFilesDropped(paths);
        }

        m_ui->layoutsView->dragLeft();
    } else if (event->mimeData()->hasText()){
        if(!event->mimeData()->text().isEmpty()){
            onRawLayoutDropped(event->mimeData()->text());
        } else if(!event->mimeData()->data("text/plain").isEmpty()) {
            onRawLayoutDropped(event->mimeData()->data("text/plain"));
        } else {
            qDebug() << "Data from drag could not be retrieved!";
        }
        m_ui->layoutsView->dragLeft();
    }
}


void TabLayouts::loadConfig()
{
    //load settings
}

void TabLayouts::saveConfig()
{
    //save settings
}

}
}
}

