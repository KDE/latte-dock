/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tablayoutshandler.h"

//! local
#include <coretypes.h>
#include "ui_settingsdialog.h"
#include "../universalsettings.h"
#include "../dialogs/detailsdialog.h"
#include "../dialogs/settingsdialog.h"
#include "../dialogs/exporttemplatedialog.h"
#include "../controllers/layoutscontroller.h"
#include "../models/layoutsmodel.h"
#include "../views/layoutstableview.h"
#include "../../apptypes.h"
#include "../../lattecorona.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/importer.h"
#include "../../layouts/manager.h"
#include "../../templates/templatesmanager.h"

//! Qt
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

    connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &TabLayouts::onCurrentPageChanged);

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

    m_pauseLayoutAction = m_layoutMenu->addAction(i18nc("pause layout", "&Pause"));
    m_pauseLayoutAction->setToolTip(i18n("Switch to selected layout"));
    m_pauseLayoutAction->setIcon(QIcon::fromTheme("media-playback-pause"));
    m_pauseLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connectActionWithButton(m_ui->pauseButton, m_pauseLayoutAction);
    connect(m_pauseLayoutAction, &QAction::triggered, this, &TabLayouts::pauseLayout);

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

    m_copyLayoutAction = m_layoutMenu->addAction(i18nc("copy layout", "&Copy"));
    m_copyLayoutAction->setToolTip(i18n("Copy selected layout"));
    m_copyLayoutAction->setIcon(QIcon::fromTheme("edit-copy"));
    m_copyLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    connectActionWithButton(m_ui->copyButton, m_copyLayoutAction);
    connect(m_copyLayoutAction, &QAction::triggered, this, &TabLayouts::copyLayout);

    m_removeLayoutAction = m_layoutMenu->addAction(i18nc("remove layout", "Remove"));
    m_removeLayoutAction->setToolTip(i18n("Remove selected layout"));
    m_removeLayoutAction->setIcon(QIcon::fromTheme("delete"));
    m_removeLayoutAction->setShortcut(QKeySequence(Qt::Key_Delete));
    connectActionWithButton(m_ui->removeButton, m_removeLayoutAction);
    connect(m_removeLayoutAction, &QAction::triggered, this, &TabLayouts::removeLayout);

    m_layoutMenu->addSeparator();

    m_readOnlyLayoutAction = m_layoutMenu->addAction(i18nc("read only layout", "&Read Only"));
    m_readOnlyLayoutAction->setToolTip(i18n("Make selected layout read-only"));
    m_readOnlyLayoutAction->setIcon(QIcon::fromTheme("object-locked"));
    m_readOnlyLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    m_readOnlyLayoutAction->setCheckable(true);
    connectActionWithButton(m_ui->readOnlyButton, m_readOnlyLayoutAction);
    connect(m_readOnlyLayoutAction, &QAction::triggered, this, &TabLayouts::lockLayout);

    m_detailsAction = m_layoutMenu->addAction(i18nc("layout details", "De&tails..."));
    m_detailsAction->setToolTip(i18n("Show selected layout details"));
    m_detailsAction->setIcon(QIcon::fromTheme("view-list-details"));
    m_detailsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    connectActionWithButton(m_ui->detailsButton, m_detailsAction);
    connect(m_detailsAction, &QAction::triggered, this, &TabLayouts::detailsLayout);

    m_layoutMenu->addSeparator();

    m_importLayoutAction = m_layoutMenu->addAction(i18nc("import layout", "&Import..."));
    m_importLayoutAction->setToolTip(i18n("Import layout file from your system"));
    m_importLayoutAction->setIcon(QIcon::fromTheme("document-import"));
    m_importLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    connectActionWithButton(m_ui->importButton, m_importLayoutAction);
    connect(m_importLayoutAction, &QAction::triggered, this, &TabLayouts::importLayout);

    m_exportLayoutAction = m_layoutMenu->addAction(i18nc("export layout", "&Export"));
    m_exportLayoutAction->setToolTip(i18n("Export selected layout at your system"));
    m_exportLayoutAction->setIcon(QIcon::fromTheme("document-export"));
    connectActionWithButton(m_ui->exportButton, m_exportLayoutAction);
    connect(m_exportLayoutAction, &QAction::triggered, m_ui->exportButton, &QPushButton::showMenu);

    initExportLayoutSubMenu();
    m_exportLayoutAction->setMenu(m_layoutExportSubMenu);
    m_ui->exportButton->setMenu(m_layoutExportSubMenu);

    m_downloadLayoutAction = m_layoutMenu->addAction(i18nc("download layout", "&Download..."));
    m_downloadLayoutAction->setToolTip(i18n("Download community layouts from KDE Store"));
    m_downloadLayoutAction->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
    m_downloadLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connectActionWithButton(m_ui->downloadButton, m_downloadLayoutAction);
    connect(m_downloadLayoutAction, &QAction::triggered, this, &TabLayouts::downloadLayout);
}

void TabLayouts::initExportLayoutSubMenu()
{
    if (!m_layoutExportSubMenu) {
        m_layoutExportSubMenu = new QMenu(m_layoutMenu);
        m_layoutExportSubMenu->setMinimumWidth(m_ui->exportButton->width() * 2);
    } else {
        m_layoutExportSubMenu->clear();
    }

    QAction *exportForBackup = m_layoutExportSubMenu->addAction(i18nc("export layout for backup","&Export For Backup..."));
    exportForBackup->setIcon(QIcon::fromTheme("document-export"));
    exportForBackup->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_E));
    connect(exportForBackup, &QAction::triggered, this, &TabLayouts::exportLayoutForBackup);

    QAction *exportAsTemplate = m_layoutExportSubMenu->addAction(i18nc("export layout as layout template","Export As &Template..."));
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
    Data::LayoutsTable templates = m_corona->templatesManager()->systemLayoutTemplates();

    for (int i=0; i<templates.rowCount(); ++i) {
        if (i==2) {
            m_layoutTemplatesSubMenu->addSeparator();
        }

        QAction *newlayout = m_layoutTemplatesSubMenu->addAction(templates[i].name);
        newlayout->setIcon(QIcon::fromTheme("document-new"));
        QString templatename = templates[i].name;

        connect(newlayout, &QAction::triggered, this, [&, templatename]() {
            newLayout(templatename);
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

bool TabLayouts::dataAreChanged() const
{
    return m_layoutsController->dataAreChanged();
}

bool TabLayouts::inDefaultValues() const
{
    return true;
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
        showInlineMessage(i18nc("settings:not permitted switching layout","You need to <b>apply</b> your changes first to switch layout..."),
                          KMessageWidget::Warning);
        return;
    }

    if (!m_layoutsController->inMultipleMode()) {
        m_corona->layoutsManager()->switchToLayout(selectedLayoutOriginal.name, MemoryUsage::SingleLayout);
        m_layoutsController->setOriginalInMultipleMode(false);
    } else {
        m_corona->layoutsManager()->switchToLayout(selectedLayoutOriginal.name, MemoryUsage::MultipleLayouts);
        m_layoutsController->setOriginalInMultipleMode(true);
    }

    updatePerLayoutButtonsState();
}

void TabLayouts::pauseLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_pauseLayoutAction->isEnabled()) {
        return;
    }

    setTwinProperty(m_pauseLayoutAction, TWINENABLED, false);

    Latte::Data::Layout selectedLayoutCurrent = m_layoutsController->selectedLayoutCurrentData();
    Latte::Data::Layout selectedLayoutOriginal = m_layoutsController->selectedLayoutOriginalData();
    selectedLayoutOriginal = selectedLayoutOriginal.isEmpty() ? selectedLayoutCurrent : selectedLayoutOriginal;

    m_corona->layoutsManager()->synchronizer()->pauseLayout(selectedLayoutOriginal.name);
}

void TabLayouts::updatePerLayoutButtonsState()
{
    //! UI Elements that need to be enabled/disabled

    //! Pause Button - visible
    if (!m_layoutsController->inMultipleMode()) {
        //! Single Layout mode
        setTwinProperty(m_pauseLayoutAction, TWINVISIBLE, false);
    } else {
        setTwinProperty(m_pauseLayoutAction, TWINVISIBLE, true);
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Latte::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    //! Switch Button
    setTwinProperty(m_switchLayoutAction, TWINENABLED, true);
    /*if (m_layoutsController->inMultipleMode()) {
        setTwinProperty(m_switchLayoutAction, TWINENABLED, false);
    } else {
        setTwinProperty(m_switchLayoutAction, TWINENABLED, true);
    }*/

    //! Pause Button - enabled
    if (m_layoutsController->inMultipleMode()) {
        if (selectedLayout.isActive
                && !selectedLayout.isOnAllActivities()
                && m_corona->layoutsManager()->synchronizer()->runningActivities().count()>1) {
            setTwinProperty(m_pauseLayoutAction, TWINENABLED, true);
        } else {
            setTwinProperty(m_pauseLayoutAction, TWINENABLED, false);
        }
    }

    //! Remove Layout Button
    /* if (selectedLayout.isActive || selectedLayout.isLocked) {
        m_ui->removeButton->setEnabled(false);
    } else {
        m_ui->removeButton->setEnabled(true);
    }*/

    //! Layout Read-Only Button
    if (selectedLayout.isLocked) {
        setTwinProperty(m_readOnlyLayoutAction, TWINCHECKED, true);
    } else {
        setTwinProperty(m_readOnlyLayoutAction, TWINCHECKED, false);
    }

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
        showInlineMessage(i18nc("settings:layout added successfully","Layout <b>%0</b> added successfully...").arg(newlayout.name),
                          KMessageWidget::Information);
    }
}

void TabLayouts::copyLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_copyLayoutAction->isEnabled()) {
        return;
    }

    m_layoutsController->copySelectedLayout();
}

void TabLayouts::downloadLayout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_downloadLayoutAction->isEnabled()) {
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
                    showInlineMessage(i18nc("settings:layout downloaded successfully","Layout <b>%0</b> downloaded successfully...").arg(downloaded.name),
                                      KMessageWidget::Information);
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

    qDebug() << Q_FUNC_INFO;

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
            showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%0</b> imported successfully...").arg(importedlayout.name),
                              KMessageWidget::Information);
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

    //! Update ALL active original layouts before exporting,
    m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();
    m_corona->universalSettings()->syncSettings();

    Dialog::ExportTemplateDialog *exportDlg = new Dialog::ExportTemplateDialog(m_parentDialog, m_layoutsController);
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
            showInlineMessage(i18nc("settings:layout export fail","Layout <b>%0</b> export <b>failed</b>...").arg(layout.name),
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

            showInlineMessage(i18nc("settings:layout export success","Layout <b>%0</b> export succeeded...").arg(selectedLayout.name),
                              KMessageWidget::Information,
                              false,
                              actions);
        } else if (file.endsWith(".latterc")) {
            auto showExportConfigurationError = [this]() {
                showInlineMessage(i18n("Full configuration export <b>failed</b>..."), KMessageWidget::Error, true);
            };

            if (m_corona->layoutsManager()->importer()->exportFullConfiguration(file)) {
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

                showInlineMessage(i18n("Full configuration export succeeded..."),
                                  KMessageWidget::Information,
                                  false,
                                  actions);
            } else {
                showExportConfigurationError();
            }
        }
    });

    exportFileDialog->open();
    exportFileDialog->selectFile(selectedLayout.name);
}

void TabLayouts::detailsLayout()
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

    detailsDlg->deleteLater();
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

    if (layoutNames.count() == 1) {
        showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%0</b> imported successfully...").arg(layoutNames[0]),
                KMessageWidget::Information);
    } else if (layoutNames.count() > 1) {
        showInlineMessage(i18nc("settings:layouts imported successfully","Layouts <b>%0</b> imported successfully...").arg(layoutNames.join(", )")),
                          KMessageWidget::Information);
    }
}

void TabLayouts::onRawLayoutDropped(const QString &rawLayout)
{
    Latte::Data::Layout importedlayout = m_layoutsController->addLayoutByText(rawLayout);
    showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%0</b> imported successfully...").arg(importedlayout.name),
                      KMessageWidget::Information);
}

bool TabLayouts::isCurrentTab() const
{
    return (m_layoutMenu->isEnabled() && (m_parentDialog->currentPage() == Dialog::LayoutPage));
}

bool TabLayouts::isHoveringLayoutsTable(const QPoint &pos)
{
    QPoint topLeft = m_ui->layoutsView->mapTo(m_parentDialog, QPoint(0,0));
    QRect geometry = m_ui->layoutsView->rect();
    geometry.moveTopLeft(topLeft);

    return geometry.contains(pos);
}


void TabLayouts::onCurrentPageChanged(int page)
{
    Dialog::ConfigurationPage cPage= static_cast<Dialog::ConfigurationPage>(page);

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

