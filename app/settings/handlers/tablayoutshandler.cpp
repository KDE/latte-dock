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
#include "ui_settingsdialog.h"
#include "../settingsdialog.h"
#include "../universalsettings.h"
#include "../controllers/layoutscontroller.h"
#include "../views/layoutstableview.h"
#include "../../lattecorona.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/importer.h"
#include "../../layouts/manager.h"
#include "../../../liblatte2/types.h"

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

TabLayouts::TabLayouts(Latte::SettingsDialog *parent)
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
    m_inMemoryButtons->addButton(m_ui->singleToolBtn, Latte::Types::SingleLayout);
    m_inMemoryButtons->addButton(m_ui->multipleToolBtn, Latte::Types::MultipleLayouts);
    m_inMemoryButtons->setExclusive(true);

    if (KWindowSystem::isPlatformWayland()) {
        m_inMemoryButtons->button(Latte::Types::MultipleLayouts)->setEnabled(false);
    }

    bool inMultiple{m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts};

    if (inMultiple) {
        m_ui->multipleToolBtn->setChecked(true);
    } else {
        m_ui->singleToolBtn->setChecked(true);
    }

    connect(m_ui->layoutsView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &Generic::dataChanged);

    connect(m_layoutsController, &Settings::Controller::Layouts::dataChanged, this, &Generic::dataChanged);

    connect(this, &Settings::Handler::TabLayouts::dataChanged, this, &TabLayouts::updatePerLayoutButtonsState);

    connect(m_inMemoryButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {

        if (checked) {
            m_layoutsController->setInMultipleMode(id == Latte::Types::MultipleLayouts);
        }
    });

    connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &TabLayouts::on_currentPageChanged);

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
    connect(m_switchLayoutAction, &QAction::triggered, this, &TabLayouts::on_switch_layout);

    m_pauseLayoutAction = m_layoutMenu->addAction(i18nc("pause layout", "&Pause"));
    m_pauseLayoutAction->setToolTip(i18n("Switch to selected layout"));
    m_pauseLayoutAction->setIcon(QIcon::fromTheme("media-playback-pause"));
    m_pauseLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    connectActionWithButton(m_ui->pauseButton, m_pauseLayoutAction);
    connect(m_pauseLayoutAction, &QAction::triggered, this, &TabLayouts::on_pause_layout);

    m_layoutMenu->addSeparator();

    m_newLayoutAction = m_layoutMenu->addAction(i18nc("new layout", "&New"));
    m_newLayoutAction->setToolTip(i18n("New layout"));
    m_newLayoutAction->setIcon(QIcon::fromTheme("add"));
    m_newLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    connectActionWithButton(m_ui->newButton, m_newLayoutAction);
    connect(m_newLayoutAction, &QAction::triggered, this, &TabLayouts::on_new_layout);

    m_copyLayoutAction = m_layoutMenu->addAction(i18nc("copy layout", "&Copy"));
    m_copyLayoutAction->setToolTip(i18n("Copy selected layout"));
    m_copyLayoutAction->setIcon(QIcon::fromTheme("edit-copy"));
    m_copyLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    connectActionWithButton(m_ui->copyButton, m_copyLayoutAction);
    connect(m_copyLayoutAction, &QAction::triggered, this, &TabLayouts::on_copy_layout);

    m_removeLayoutAction = m_layoutMenu->addAction(i18nc("remove layout", "Remove"));
    m_removeLayoutAction->setToolTip(i18n("Remove selected layout"));
    m_removeLayoutAction->setIcon(QIcon::fromTheme("delete"));
    m_removeLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    connectActionWithButton(m_ui->removeButton, m_removeLayoutAction);
    connect(m_removeLayoutAction, &QAction::triggered, this, &TabLayouts::on_remove_layout);

    m_layoutMenu->addSeparator();

    m_lockedLayoutAction = m_layoutMenu->addAction(i18nc("locked layout", "&Locked"));
    m_lockedLayoutAction->setToolTip(i18n("Lock/Unlock selected layout and make it read-only"));
    m_lockedLayoutAction->setIcon(QIcon::fromTheme("object-locked"));
    m_lockedLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    m_lockedLayoutAction->setCheckable(true);
    connectActionWithButton(m_ui->lockedButton, m_lockedLayoutAction);
    connect(m_lockedLayoutAction, &QAction::triggered, this, &TabLayouts::on_locked_layout);

    m_sharedLayoutAction = m_layoutMenu->addAction(i18nc("shared layout", "Sha&red"));
    m_sharedLayoutAction->setToolTip(i18n("Share selected layout with other central layouts"));
    m_sharedLayoutAction->setIcon(QIcon::fromTheme("document-share"));
    m_sharedLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    m_sharedLayoutAction->setCheckable(true);
    connectActionWithButton(m_ui->sharedButton, m_sharedLayoutAction);
    connect(m_sharedLayoutAction, &QAction::triggered, this, &TabLayouts::on_shared_layout);

    m_detailsAction = m_layoutMenu->addAction(i18nc("layout details", "De&tails..."));
    m_detailsAction->setToolTip(i18n("Show selected layout details"));
    m_detailsAction->setIcon(QIcon::fromTheme("view-list-details"));
    m_detailsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
    connectActionWithButton(m_ui->detailsButton, m_detailsAction);
    connect(m_detailsAction, &QAction::triggered, this, &TabLayouts::on_details_action);

    m_layoutMenu->addSeparator();

    m_importLayoutAction = m_layoutMenu->addAction(i18nc("import layout", "&Import..."));
    m_importLayoutAction->setToolTip(i18n("Import layout file from your system"));
    m_importLayoutAction->setIcon(QIcon::fromTheme("document-import"));
    m_importLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
    connectActionWithButton(m_ui->importButton, m_importLayoutAction);
    connect(m_importLayoutAction, &QAction::triggered, this, &TabLayouts::on_import_layout);

    m_exportLayoutAction = m_layoutMenu->addAction(i18nc("export layout", "&Export..."));
    m_exportLayoutAction->setToolTip(i18n("Export selected layout at your system"));
    m_exportLayoutAction->setIcon(QIcon::fromTheme("document-export"));
    m_exportLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_E));
    connectActionWithButton(m_ui->exportButton, m_exportLayoutAction);
    connect(m_exportLayoutAction, &QAction::triggered, this, &TabLayouts::on_export_layout);

    m_downloadLayoutAction = m_layoutMenu->addAction(i18nc("download layout", "&Download..."));
    m_downloadLayoutAction->setToolTip(i18n("Download community layouts from KDE Store"));
    m_downloadLayoutAction->setIcon(QIcon::fromTheme("get-hot-new-stuff"));
    m_downloadLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_D));
    connectActionWithButton(m_ui->downloadButton, m_downloadLayoutAction);
    connect(m_downloadLayoutAction, &QAction::triggered, this, &TabLayouts::on_download_layout);
}

Latte::Corona *TabLayouts::corona() const
{
    return m_corona;
}

Latte::SettingsDialog *TabLayouts::dialog() const
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
}

void TabLayouts::resetDefaults()
{
    //do nothing because there are no defaults
}

void TabLayouts::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval, QList<QAction *> actions)
{
    m_parentDialog->showInlineMessage(msg, type, hideInterval, actions);
}

void TabLayouts::save()
{
    m_layoutsController->save();
}

void TabLayouts::on_switch_layout()
{
    if (!isCurrentTab() || !m_switchLayoutAction->isEnabled()) {
        return;
    }

    Settings::Data::Layout selectedLayoutCurrent = m_layoutsController->selectedLayoutCurrentData();
    Settings::Data::Layout selectedLayoutOriginal = m_layoutsController->selectedLayoutOriginalData();
    selectedLayoutOriginal = selectedLayoutOriginal.isEmpty() ? selectedLayoutCurrent : selectedLayoutOriginal;

    if (m_layoutsController->dataAreChanged()) {
        showInlineMessage(i18nc("settings:not permitted switching layout","You need to <b>apply</b> your changes first to switch layout..."),
                          KMessageWidget::Warning,
                          SettingsDialog::WARNINGINTERVAL);
        return;
    }

    if (!m_layoutsController->selectedLayoutIsCurrentActive()) {
        bool appliedShared = m_layoutsController->inMultipleMode() && selectedLayoutCurrent.isShared();
        bool freeActivitiesLayoutUpdated{false};

        if (!appliedShared && selectedLayoutCurrent.activities.isEmpty()) {
            m_layoutsController->setOriginalLayoutForFreeActivities(selectedLayoutOriginal.id);
            freeActivitiesLayoutUpdated = true;
        }

        if (m_layoutsController->inMultipleMode()) {
            m_corona->layoutsManager()->switchToLayout(selectedLayoutOriginal.name);
        } else {
            if (freeActivitiesLayoutUpdated) {
                m_corona->layoutsManager()->switchToLayout(selectedLayoutOriginal.name);
            } else {
                CentralLayout singleLayout(this, selectedLayoutCurrent.id);

                QString switchToActivity = selectedLayoutCurrent.isForFreeActivities() ? singleLayout.lastUsedActivity() : selectedLayoutCurrent.activities[0];

                if (!m_corona->activitiesConsumer()->runningActivities().contains(switchToActivity)) {
                    m_corona->layoutsManager()->synchronizer()->activitiesController()->startActivity(switchToActivity);
                }

                m_corona->layoutsManager()->synchronizer()->activitiesController()->setCurrentActivity(switchToActivity);
            }
        }
    }

    updatePerLayoutButtonsState();
}

void TabLayouts::on_pause_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_pauseLayoutAction->isEnabled()) {
        return;
    }

    setTwinProperty(m_pauseLayoutAction, TWINENABLED, false);

    Settings::Data::Layout selectedLayoutCurrent = m_layoutsController->selectedLayoutCurrentData();
    Settings::Data::Layout selectedLayoutOriginal = m_layoutsController->selectedLayoutOriginalData();
    selectedLayoutOriginal = selectedLayoutOriginal.isEmpty() ? selectedLayoutCurrent : selectedLayoutOriginal;

    m_corona->layoutsManager()->synchronizer()->pauseLayout(selectedLayoutOriginal.name);
}

void TabLayouts::updatePerLayoutButtonsState()
{
    //! UI Elements that need to be enabled/disabled

    //! Shared Button - visible
    if (m_layoutsController->inMultipleMode()) {
        setTwinProperty(m_sharedLayoutAction, TWINVISIBLE, true);
    } else {
        setTwinProperty(m_sharedLayoutAction, TWINVISIBLE, false);
    }

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

    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    //! Switch Button
    if ((m_layoutsController->inMultipleMode() && selectedLayout.isShared())
            || m_layoutsController->selectedLayoutIsCurrentActive()) {
        setTwinProperty(m_switchLayoutAction, TWINENABLED, false);
    } else {
        setTwinProperty(m_switchLayoutAction, TWINENABLED, true);
    }

    //! Pause Button - enabled
    if (m_layoutsController->inMultipleMode()) {
        if (selectedLayout.isActive
                && !selectedLayout.isForFreeActivities()
                && !selectedLayout.isShared()) {
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

    //! Layout Locked Button
    if (selectedLayout.isLocked) {
        setTwinProperty(m_lockedLayoutAction, TWINCHECKED, true);
    } else {
        setTwinProperty(m_lockedLayoutAction, TWINCHECKED, false);
    }

    //! Layout Shared Button
    if (selectedLayout.isShared()) {
        setTwinProperty(m_sharedLayoutAction, TWINCHECKED, true);
    } else {
        setTwinProperty(m_sharedLayoutAction, TWINCHECKED, false);
    }

    setTwinProperty(m_detailsAction, TWINENABLED, true);
}

void TabLayouts::on_new_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_newLayoutAction->isEnabled()) {
        return;
    }

    //! find Default preset path
    for (const auto &preset : m_corona->layoutsManager()->presetsPaths()) {
        QString presetName = CentralLayout::layoutName(preset);

        if (presetName == "Default") {
            Settings::Data::Layout newlayout = m_layoutsController->addLayoutForFile(preset, presetName, true);
            showInlineMessage(i18nc("settings:layout added successfully","Layout <b>%0</b> added successfully...").arg(newlayout.name),
                              KMessageWidget::Information,
                              SettingsDialog::INFORMATIONINTERVAL);
            break;
        }
    }
}

void TabLayouts::on_copy_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_copyLayoutAction->isEnabled()) {
        return;
    }

    m_layoutsController->copySelectedLayout();
}

void TabLayouts::on_download_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_downloadLayoutAction->isEnabled()) {
        return;
    }

    KNS3::DownloadDialog dialog(QStringLiteral("latte-layouts.knsrc"), m_parentDialog);
    dialog.resize(m_parentDialog->downloadWindowSize());
    dialog.exec();

    if (!dialog.changedEntries().isEmpty() || !dialog.installedEntries().isEmpty()) {
        for (const auto &entry : dialog.installedEntries()) {
            for (const auto &entryFile : entry.installedFiles()) {
                Latte::Layouts::Importer::LatteFileVersion version = Latte::Layouts::Importer::fileVersion(entryFile);

                if (version == Latte::Layouts::Importer::LayoutVersion2) {
                    Settings::Data::Layout downloaded = m_layoutsController->addLayoutForFile(entryFile);
                    showInlineMessage(i18nc("settings:layout downloaded successfully","Layout <b>%0</b> downloaded successfully...").arg(downloaded.name),
                                      KMessageWidget::Information,
                                      SettingsDialog::INFORMATIONINTERVAL);
                    break;
                }
            }
        }
    }

    m_parentDialog->setDownloadWindowSize(dialog.size());
}

void TabLayouts::on_remove_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_removeLayoutAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    if (selectedLayout.isActive) {
        showInlineMessage(i18nc("settings: active layout remove","<b>Active</b> layouts can not be removed..."),
                          KMessageWidget::Error,
                          SettingsDialog::WARNINGINTERVAL);
        return;
    }

    if (selectedLayout.isLocked) {
        showInlineMessage(i18nc("settings: locked layout remove","Locked layouts can not be removed..."),
                          KMessageWidget::Error,
                          SettingsDialog::WARNINGINTERVAL);
        return;
    }

    qDebug() << Q_FUNC_INFO;

    m_layoutsController->removeSelected();
}

void TabLayouts::on_locked_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_lockedLayoutAction->isEnabled()) {
        return;
    }

    m_layoutsController->toggleLockedForSelected();

    updatePerLayoutButtonsState();
}

void TabLayouts::on_shared_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_sharedLayoutAction->isEnabled()) {
        return;
    }

    m_layoutsController->toggleSharedForSelected();

    updatePerLayoutButtonsState();
}

void TabLayouts::on_import_layout()
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
            Settings::Data::Layout importedlayout = m_layoutsController->addLayoutForFile(file);
            showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%0</b> imported successfully...").arg(importedlayout.name),
                              KMessageWidget::Information,
                              SettingsDialog::INFORMATIONINTERVAL);
        } else if (version == Latte::Layouts::Importer::ConfigVersion1) {
            if (!m_layoutsController->importLayoutsFromV1ConfigFile(file)) {
                showInlineMessage(i18nc("settings:deprecated layouts import failed","Import layouts from deprecated version <b>failed</b>..."),
                                  KMessageWidget::Error);
            }
        }
    });

    importFileDialog->open();
}

void TabLayouts::on_export_layout()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_exportLayoutAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();

    //! Update ALL active original layouts before exporting,
    m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();

    QFileDialog *exportFileDialog = new QFileDialog(m_parentDialog, i18n("Export Layout"), QDir::homePath(), QStringLiteral("layout.latte"));

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
        auto showExportLayoutError = [this](const Settings::Data::Layout &layout) {
            showInlineMessage(i18nc("settings:layout export fail","Layout <b>%0</b> export <b>failed</b>...").arg(layout.name),
                              KMessageWidget::Error);
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
                              SettingsDialog::INFORMATIONWITHACTIONINTERVAL,
                              actions);
        } else if (file.endsWith(".latterc")) {
            auto showExportConfigurationError = [this]() {
                showInlineMessage(i18n("Full configuration export <b>failed</b>..."), KMessageWidget::Error);
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
                                  SettingsDialog::INFORMATIONWITHACTIONINTERVAL,
                                  actions);
            } else {
                showExportConfigurationError();
            }
        }
    });

    exportFileDialog->open();
    exportFileDialog->selectFile(selectedLayout.name);
}

void TabLayouts::on_details_action()
{
    qDebug() << Q_FUNC_INFO;

    if (!isCurrentTab() || !m_exportLayoutAction->isEnabled()) {
        return;
    }

    if (!m_layoutsController->hasSelectedLayout()) {
        return;
    }

    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayoutCurrentData();
}

void TabLayouts::on_layoutFilesDropped(const QStringList &paths)
{
    QStringList layoutNames;

    for (int i=0; i<paths.count(); ++i) {
        if (paths[i].endsWith(".layout.latte")) {
            Settings::Data::Layout importedlayout = m_layoutsController->addLayoutForFile(paths[i]);
            layoutNames << importedlayout.name;
        }
    }

    if (layoutNames.count() == 1) {
        showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%0</b> imported successfully...").arg(layoutNames[0]),
                KMessageWidget::Information,
                SettingsDialog::INFORMATIONINTERVAL);
    } else if (layoutNames.count() > 1) {
        showInlineMessage(i18nc("settings:layouts imported successfully","Layouts <b>%0</b> imported successfully...").arg(layoutNames.join(", )")),
                          KMessageWidget::Information,
                          SettingsDialog::INFORMATIONINTERVAL);
    }
}

bool TabLayouts::isCurrentTab() const
{
    return (m_layoutMenu->isEnabled() && (m_parentDialog->currentPage() == Types::LayoutPage));
}

bool TabLayouts::isHoveringLayoutsTable(const QPoint &pos)
{
    QPoint topLeft = m_ui->layoutsView->mapTo(m_parentDialog, QPoint(0,0));
    QRect geometry = m_ui->layoutsView->rect();
    geometry.moveTopLeft(topLeft);

    return geometry.contains(pos);
}


void TabLayouts::on_currentPageChanged(int page)
{
    Types::LatteConfigPage cPage= static_cast<Types::LatteConfigPage>(page);

    if (cPage == Types::LayoutPage) {
        m_layoutMenu->setEnabled(true);
        m_layoutMenu->menuAction()->setVisible(true);

    } else {
        m_layoutMenu->menuAction()->setVisible(false);
        m_layoutMenu->setEnabled(false);
    }
}

void TabLayouts::on_dragEnterEvent(QDragEnterEvent *event)
{
    if (!isHoveringLayoutsTable(event->pos())) {
        return;
    }

    event->acceptProposedAction();
    m_ui->layoutsView->dragEntered();
}

void TabLayouts::on_dragLeaveEvent(QDragLeaveEvent *event)
{
    m_ui->layoutsView->dragLeft();
}

void TabLayouts::on_dragMoveEvent(QDragMoveEvent *event)
{
    if (!isHoveringLayoutsTable(event->pos())) {
        event->ignore();
        m_ui->layoutsView->dragLeft();
        return;
    }

    event->acceptProposedAction();
    m_ui->layoutsView->dragEntered();
}

void TabLayouts::on_dropEvent(QDropEvent *event)
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
            on_layoutFilesDropped(paths);
        }

        m_ui->layoutsView->dragLeft();
    }
}

void TabLayouts::on_keyReleaseEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Delete && m_parentDialog->currentPage() == Types::LayoutPage){
        on_remove_layout();
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

