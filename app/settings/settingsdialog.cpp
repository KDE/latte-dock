/*
 * Copyright 2017  Smith AR <audoban@openmailbox.org>
 *                 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "settingsdialog.h"

// local
#include "universalsettings.h"
#include "ui_settingsdialog.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layout/centrallayout.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../layouts/synchronizer.h"
#include "../liblatte2/types.h"
#include "../plasma/extended/theme.h"
#include "data/layoutdata.h"
#include "tools/settingstools.h"

// Qt
#include <QButtonGroup>
#include <QColorDialog>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>

// KDE
#include <KActivities/Controller>
#include <KLocalizedString>
#include <KNotification>
#include <KWindowSystem>
#include <KNewStuff3/KNS3/DownloadDialog>

namespace Latte {

const int SCREENTRACKERDEFAULTVALUE = 2500;
const int OUTLINEDEFAULTWIDTH = 1;


SettingsDialog::SettingsDialog(QWidget *parent, Latte::Corona *corona)
    : QDialog(parent),
      ui(new Ui::SettingsDialog),
      m_corona(corona)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    resize(m_corona->universalSettings()->layoutsWindowSize());

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked
            , this, &SettingsDialog::apply);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked
            , this, &SettingsDialog::cancel);
    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked
            , this, &SettingsDialog::ok);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &SettingsDialog::restoreDefaults);

    m_layoutsController = new Settings::Controller::Layouts(this, m_corona, ui->layoutsView);
    m_model = m_layoutsController->model();

    //connect(m_corona->layoutsManager(), &Layouts::Manager::currentLayoutNameChanged, this, &SettingsDialog::layoutsChanged);
    //connect(m_corona->layoutsManager(), &Layouts::Manager::centralLayoutsChanged, this, &SettingsDialog::layoutsChanged);


    m_inMemoryButtons = new QButtonGroup(this);
    m_inMemoryButtons->addButton(ui->singleToolBtn, Latte::Types::SingleLayout);
    m_inMemoryButtons->addButton(ui->multipleToolBtn, Latte::Types::MultipleLayouts);
    m_inMemoryButtons->setExclusive(true);

    if (KWindowSystem::isPlatformWayland()) {
        m_inMemoryButtons->button(Latte::Types::MultipleLayouts)->setEnabled(false);
    }

    m_mouseSensitivityButtons = new QButtonGroup(this);
    m_mouseSensitivityButtons->addButton(ui->lowSensitivityBtn, Latte::Types::LowSensitivity);
    m_mouseSensitivityButtons->addButton(ui->mediumSensitivityBtn, Latte::Types::MediumSensitivity);
    m_mouseSensitivityButtons->addButton(ui->highSensitivityBtn, Latte::Types::HighSensitivity);
    m_mouseSensitivityButtons->setExclusive(true);

    ui->screenTrackerSpinBox->setValue(m_corona->universalSettings()->screenTrackerInterval());
    ui->outlineSpinBox->setValue(m_corona->themeExtended()->outlineWidth());

    //! About Menu
    QMenuBar *menuBar = new QMenuBar(this);
    // QMenuBar *rightAlignedMenuBar = new QMenuBar(menuBar);

    layout()->setMenuBar(menuBar);
    //menuBar->setCornerWidget(rightAlignedMenuBar);

    QMenu *fileMenu = new QMenu(i18n("File"), menuBar);
    menuBar->addMenu(fileMenu);

    QMenu *layoutMenu = new QMenu(i18n("Layout"), menuBar);
    menuBar->addMenu(layoutMenu);

    //! Help menu
    m_helpMenu = new KHelpMenu(menuBar);
    menuBar->addMenu(m_helpMenu->menu());
    //rightAlignedMenuBar->addMenu(helpMenu);

    //! hide help menu actions that are not used
    m_helpMenu->action(KHelpMenu::menuHelpContents)->setVisible(false);
    m_helpMenu->action(KHelpMenu::menuWhatsThis)->setVisible(false);


    QAction *screensAction = fileMenu->addAction(i18n("Sc&reens..."));
    screensAction->setIcon(QIcon::fromTheme("document-properties"));
    screensAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));

    QAction *quitAction = fileMenu->addAction(i18n("&Quit Latte"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    m_editLayoutAction = layoutMenu->addAction(i18nc("edit layout","&Edit..."));
    m_editLayoutAction->setIcon(QIcon::fromTheme("document-edit"));
    m_editLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    m_editLayoutAction->setToolTip("You can edit layout file when layout is not active or locked");

    QAction *infoLayoutAction = layoutMenu->addAction(i18nc("layout information","&Information..."));
    infoLayoutAction->setIcon(QIcon::fromTheme("document-properties"));
    infoLayoutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));

    //! RTL support for labels in preferences
    if (qApp->layoutDirection() == Qt::RightToLeft) {
        ui->behaviorLbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
        ui->mouseSensetivityLbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
        ui->delayLbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
    }

    loadSettings();

    //! SIGNALS
    connect(ui->layoutsView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [&]() {
        updatePerLayoutButtonsState();
        updateApplyButtonsState();
    });

    connect(m_inMemoryButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {

        if (checked) {
            m_layoutsController->setInMultipleMode(id == Latte::Types::MultipleLayouts);

            updateApplyButtonsState();
            updatePerLayoutButtonsState();
            updateSharedLayoutsUiElements();
        }
    });

    connect(m_mouseSensitivityButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
            [ = ](int id, bool checked) {
        updateApplyButtonsState();
    });

    connect(ui->screenTrackerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        updateApplyButtonsState();
    });

    connect(ui->outlineSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        updateApplyButtonsState();
    });

    connect(ui->autostartChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->badges3DStyleChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->metaPressChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->metaPressHoldChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->infoWindowChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsDialog::updateApplyButtonsState);

    connect(ui->noBordersForMaximizedChkBox, &QCheckBox::stateChanged, this, [&]() {
        bool noBordersForMaximized = ui->noBordersForMaximizedChkBox->isChecked();

        if (noBordersForMaximized) {
            ui->layoutsView->setColumnHidden(Settings::Model::Layouts::BORDERSCOLUMN, false);
        } else {
            ui->layoutsView->setColumnHidden(Settings::Model::Layouts::BORDERSCOLUMN, true);
        }

        updateApplyButtonsState();
    });

    connect(quitAction, &QAction::triggered, this, [&]() {
        close();
        m_corona->quitApplication();
    });

    connect(m_editLayoutAction, &QAction::triggered, this, [&]() {
        QString file = m_layoutsController->selectedLayout().id;

        if (!file.isEmpty()) {
            QProcess::startDetached("kwrite \"" + file + "\"");
        }
    });

    connect(infoLayoutAction, &QAction::triggered, this, &SettingsDialog::showLayoutInformation);
    connect(screensAction, &QAction::triggered, this, &SettingsDialog::showScreensInformation);

    blockDeleteOnActivityStopped();
}

SettingsDialog::~SettingsDialog()
{
    qDebug() << Q_FUNC_INFO;

    m_corona->universalSettings()->setLayoutsWindowSize(size());
}

void SettingsDialog::blockDeleteOnActivityStopped()
{
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged,
            this, [&]() {
        m_blockDeleteOnReject = true;
        m_activityClosedTimer.start();
    });

    m_activityClosedTimer.setSingleShot(true);
    m_activityClosedTimer.setInterval(500);
    connect(&m_activityClosedTimer, &QTimer::timeout, this, [&]() {
        m_blockDeleteOnReject = false;
    });
}

void SettingsDialog::toggleCurrentPage()
{
    if (ui->tabWidget->currentIndex() == 0) {
        ui->tabWidget->setCurrentIndex(1);
    } else {
        ui->tabWidget->setCurrentIndex(0);
    }                                   
}

void SettingsDialog::setCurrentPage(int page)
{
    ui->tabWidget->setCurrentIndex(page);
}

void SettingsDialog::on_newButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    //! find Default preset path
    for (const auto &preset : m_corona->layoutsManager()->presetsPaths()) {
        QString presetName = CentralLayout::layoutName(preset);

        if (presetName == "Default") {
            m_layoutsController->addLayoutForFile(preset, presetName, true, false);
            break;
        }
    }
}

void SettingsDialog::on_copyButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    m_layoutsController->copySelectedLayout();
}

void SettingsDialog::on_downloadButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    KNS3::DownloadDialog dialog(QStringLiteral("latte-layouts.knsrc"), this);
    dialog.resize(m_corona->universalSettings()->downloadWindowSize());
    dialog.exec();

    bool layoutAdded{false};

    if (!dialog.changedEntries().isEmpty() || !dialog.installedEntries().isEmpty()) {
        for (const auto &entry : dialog.installedEntries()) {
            for (const auto &entryFile : entry.installedFiles()) {
                Layouts::Importer::LatteFileVersion version = Layouts::Importer::fileVersion(entryFile);

                if (version == Layouts::Importer::LayoutVersion2) {
                    layoutAdded = true;
                    m_layoutsController->addLayoutForFile(entryFile);
                    break;
                }
            }
        }
    }

    m_corona->universalSettings()->setDownloadWindowSize(dialog.size());

    if (layoutAdded) {
        apply();
    }
}

void SettingsDialog::on_removeButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    m_layoutsController->removeSelected();

    updateApplyButtonsState();
}

void SettingsDialog::on_lockedButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    m_layoutsController->toggleLockedForSelected();

    updatePerLayoutButtonsState();
    updateApplyButtonsState();
}

void SettingsDialog::on_sharedButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    m_layoutsController->toggleSharedForSelected();

    updatePerLayoutButtonsState();
    updateApplyButtonsState();
}

void SettingsDialog::on_importButton_clicked()
{
    qDebug() << Q_FUNC_INFO;


    QFileDialog *fileDialog = new QFileDialog(this, i18nc("import layout/configuration", "Import Layout/Configuration")
                                              , QDir::homePath()
                                              , QStringLiteral("layout.latte"));

    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setDefaultSuffix("layout.latte");

    QStringList filters;
    filters << QString(i18nc("import latte layout", "Latte Dock Layout file v0.2") + "(*.layout.latte)")
            << QString(i18nc("import latte layouts/configuration", "Latte Dock Full Configuration file (v0.1, v0.2)") + "(*.latterc)");
    fileDialog->setNameFilters(filters);

    connect(fileDialog, &QFileDialog::finished
            , fileDialog, &QFileDialog::deleteLater);

    connect(fileDialog, &QFileDialog::fileSelected
            , this, [&](const QString & file) {
        Layouts::Importer::LatteFileVersion version = Layouts::Importer::fileVersion(file);
        qDebug() << "VERSION :::: " << version;

        if (version == Layouts::Importer::LayoutVersion2) {
            m_layoutsController->addLayoutForFile(file);
        } else if (version == Layouts::Importer::ConfigVersion1) {
            auto msg = new QMessageBox(this);
            msg->setIcon(QMessageBox::Warning);
            msg->setWindowTitle(i18n("Import: Configuration file version v0.1"));
            msg->setText(
                        i18n("You are going to import an old version <b>v0.1</b> configuration file.<br><b>Be careful</b>, importing the entire configuration <b>will erase all</b> your current configuration!!!<br><br> <i>Alternative, you can <b>import safely</b> from this file<br><b>only the contained layouts...</b></i>"));
            msg->setStandardButtons(QMessageBox::Cancel);

            QPushButton *fullBtn = new QPushButton(msg);
            QPushButton *layoutsBtn = new QPushButton(msg);
            fullBtn->setText(i18nc("import full configuration", "Full Configuration"));
            fullBtn->setIcon(QIcon::fromTheme("settings"));
            layoutsBtn->setText(i18nc("import only the layouts", "Only Layouts"));
            layoutsBtn->setIcon(QIcon::fromTheme("user-identity"));

            msg->addButton(fullBtn, QMessageBox::AcceptRole);
            msg->addButton(layoutsBtn, QMessageBox::AcceptRole);

            msg->setDefaultButton(layoutsBtn);

            connect(msg, &QMessageBox::finished, msg, &QMessageBox::deleteLater);

            msg->open();

            connect(layoutsBtn, &QPushButton::clicked
                    , this, [ &, file](bool check) {
                m_layoutsController->importLayoutsFromV1ConfigFile(file);
            });

            connect(fullBtn, &QPushButton::clicked
                    , this, [ &, file](bool check) {
                //!NOTE: Restart latte for import the new configuration
                QProcess::startDetached(qGuiApp->applicationFilePath() + " --import-full \"" + file + "\"");
                qGuiApp->exit();
            });
        } else if (version == Layouts::Importer::ConfigVersion2) {
            auto msg = new QMessageBox(this);
            msg->setIcon(QMessageBox::Warning);
            msg->setWindowTitle(i18n("Import: Configuration file version v0.2"));
            msg->setText(
                        i18n("You are going to import a <b>v0.2</b> configuration file.<br><b>Be careful</b>, importing <b>will erase all</b> your current configuration!!!<br><br><i>Would you like to proceed?</i>"));
            msg->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msg->setDefaultButton(QMessageBox::No);

            connect(msg, &QMessageBox::finished, this, [ &, msg, file](int result) {
                if (result == QMessageBox::Yes) {
                    //!NOTE: Restart latte for import the new configuration
                    msg->deleteLater();
                    QProcess::startDetached(qGuiApp->applicationFilePath() + " --import-full \"" + file + "\"");
                    qGuiApp->exit();
                }
            });

            msg->open();
        }
    });

    fileDialog->open();
}

void SettingsDialog::on_exportButton_clicked()
{
    if (ui->layoutsView->currentIndex().row() < 0) {
        return;
    }

    QString layoutExported = m_layoutsController->selectedLayout().id;

    //! Update ALL active original layouts before exporting,
    //! this is needed because the export method can export also the full configuration
    qDebug() << Q_FUNC_INFO;

    m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();

    QFileDialog *fileDialog = new QFileDialog(this, i18nc("export layout/configuration", "Export Layout/Configuration")
                                              , QDir::homePath(), QStringLiteral("layout.latte"));

    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setDefaultSuffix("layout.latte");

    QStringList filters;
    QString filter1(i18nc("export layout", "Latte Dock Layout file v0.2") + "(*.layout.latte)");
    QString filter2(i18nc("export full configuration", "Latte Dock Full Configuration file v0.2") + "(*.latterc)");

    filters << filter1
            << filter2;

    fileDialog->setNameFilters(filters);

    connect(fileDialog, &QFileDialog::finished
            , fileDialog, &QFileDialog::deleteLater);

    connect(fileDialog, &QFileDialog::fileSelected
            , this, [ &, layoutExported](const QString & file) {
        auto showNotificationError = []() {
            auto notification = new KNotification("export-fail", KNotification::CloseOnTimeout);
            notification->setText(i18nc("export layout", "Failed to export layout"));
            notification->sendEvent();
        };

        if (QFile::exists(file) && !QFile::remove(file)) {
            showNotificationError();
            return;
        }

        if (file.endsWith(".layout.latte")) {
            if (!QFile(layoutExported).copy(file)) {
                showNotificationError();
                return;
            }

            QFileInfo newFileInfo(file);

            if (newFileInfo.exists() && !newFileInfo.isWritable()) {
                QFile(file).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
            }

            CentralLayout layoutS(this, file);
            layoutS.setActivities(QStringList());
            layoutS.clearLastUsedActivity();

            //NOTE: The pointer is automatically deleted when the event is closed
            auto notification = new KNotification("export-done", KNotification::CloseOnTimeout);
            notification->setActions({i18nc("export layout", "Open location")});
            notification->setText(i18nc("export layout", "Layout exported successfully"));

            connect(notification, &KNotification::action1Activated
                    , this, [file]() {
                QDesktopServices::openUrl({QFileInfo(file).canonicalPath()});
            });

            notification->sendEvent();
        } else if (file.endsWith(".latterc")) {
            auto showNotificationError = []() {
                auto notification = new KNotification("export-fail", KNotification::CloseOnTimeout);
                notification->setText(i18nc("import/export config", "Failed to export configuration"));
                notification->sendEvent();
            };

            if (m_corona->layoutsManager()->importer()->exportFullConfiguration(file)) {

                auto notification = new KNotification("export-done", KNotification::CloseOnTimeout);
                notification->setActions({i18nc("import/export config", "Open location")});
                notification->setText(i18nc("import/export config", "Full Configuration exported successfully"));

                connect(notification, &KNotification::action1Activated
                        , this, [file]() {
                    QDesktopServices::openUrl({QFileInfo(file).canonicalPath()});
                });

                notification->sendEvent();
            } else {
                showNotificationError();
            }
        }
    });


    fileDialog->open();
}

void SettingsDialog::requestImagesDialog(int row)
{
    QStringList mimeTypeFilters;
    mimeTypeFilters << "image/jpeg" // will show "JPEG image (*.jpeg *.jpg)
                    << "image/png";  // will show "PNG image (*.png)"

    QFileDialog dialog(this);
    dialog.setMimeTypeFilters(mimeTypeFilters);

    QString background = "";// m_model->data(m_model->index(row, COLORCOLUMN), Qt::BackgroundRole).toString();

    if (background.startsWith("/") && QFileInfo(background).exists()) {
        dialog.setDirectory(QFileInfo(background).absolutePath());
        dialog.selectFile(background);
    }

    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();

        if (files.count() > 0) {
           // m_model->setData(m_model->index(row, COLORCOLUMN), files[0], Qt::BackgroundRole);
        }
    }
}

void SettingsDialog::requestColorsDialog(int row)
{
    QColorDialog dialog(this);
    QString textColor = m_model->data(m_model->index(row, Settings::Model::Layouts::BACKGROUNDCOLUMN), Qt::UserRole).toString();
    dialog.setCurrentColor(QColor(textColor));

    if (dialog.exec()) {
        qDebug() << dialog.selectedColor().name();
        //m_model->setData(m_model->index(row, COLORCOLUMN), dialog.selectedColor().name(), Qt::UserRole);
    }
}

void SettingsDialog::accept()
{
    //! disable accept totally in order to avoid closing with ENTER key with no real reason
    qDebug() << Q_FUNC_INFO;
}


void SettingsDialog::cancel()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_blockDeleteOnReject) {
        deleteLater();
    }
}

void SettingsDialog::ok()
{
    if (!ui->buttonBox->button(QDialogButtonBox::Ok)->hasFocus()) {
        return;
    }

    qDebug() << Q_FUNC_INFO;

    saveAllChanges();
    deleteLater();
}

void SettingsDialog::apply()
{
    qDebug() << Q_FUNC_INFO;

    saveAllChanges();

    updateApplyButtonsState();
    updatePerLayoutButtonsState();
}

void SettingsDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;

    if (ui->tabWidget->currentIndex() == 0) {
        //! do nothing, should be disabled
    } else if (ui->tabWidget->currentIndex() == 1) {
        //! Defaults for general Latte settings
        ui->autostartChkBox->setChecked(true);
        ui->badges3DStyleChkBox->setChecked(true);
        ui->infoWindowChkBox->setChecked(true);
        ui->metaPressChkBox->setChecked(false);
        ui->metaPressHoldChkBox->setChecked(true);
        ui->noBordersForMaximizedChkBox->setChecked(false);
        ui->highSensitivityBtn->setChecked(true);
        ui->screenTrackerSpinBox->setValue(SCREENTRACKERDEFAULTVALUE);
        ui->outlineSpinBox->setValue(OUTLINEDEFAULTWIDTH);
    }
}

void SettingsDialog::loadSettings()
{
    bool inMultiple{m_corona->layoutsManager()->memoryUsage() == Types::MultipleLayouts};

    m_layoutsController->loadLayouts();
    m_layoutsController->setOriginalInMultipleMode(inMultiple);

    if (inMultiple) {
        ui->multipleToolBtn->setChecked(true);
    } else {
        ui->singleToolBtn->setChecked(true);
    }

    updatePerLayoutButtonsState();

    ui->autostartChkBox->setChecked(m_corona->universalSettings()->autostart());
    ui->badges3DStyleChkBox->setChecked(m_corona->universalSettings()->badges3DStyle());
    ui->infoWindowChkBox->setChecked(m_corona->universalSettings()->showInfoWindow());
    ui->metaPressChkBox->setChecked(m_corona->universalSettings()->kwin_metaForwardedToLatte());
    ui->metaPressHoldChkBox->setChecked(m_corona->universalSettings()->metaPressAndHoldEnabled());
    ui->noBordersForMaximizedChkBox->setChecked(m_corona->universalSettings()->canDisableBorders());

    if (m_corona->universalSettings()->mouseSensitivity() == Types::LowSensitivity) {
        ui->lowSensitivityBtn->setChecked(true);
    } else if (m_corona->universalSettings()->mouseSensitivity() == Types::MediumSensitivity) {
        ui->mediumSensitivityBtn->setChecked(true);
    } else if (m_corona->universalSettings()->mouseSensitivity() == Types::HighSensitivity) {
        ui->highSensitivityBtn->setChecked(true);
    }

    o_settingsOriginalData = currentSettings();
    updateApplyButtonsState();
    updateSharedLayoutsUiElements();
}

QList<int> SettingsDialog::currentSettings()
{
    QList<int> settings;
    settings << (int)ui->autostartChkBox->isChecked();
    settings << (int)ui->badges3DStyleChkBox->isChecked();
    settings << (int)ui->infoWindowChkBox->isChecked();
    settings << (int)ui->metaPressChkBox->isChecked();
    settings << (int)ui->metaPressHoldChkBox->isChecked();
    settings << (int)ui->noBordersForMaximizedChkBox->isChecked();
    settings << m_mouseSensitivityButtons->checkedId();
    settings << ui->screenTrackerSpinBox->value();
    settings << ui->outlineSpinBox->value();

    return settings;
}

void SettingsDialog::on_switchButton_clicked()
{
    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayout();

    if (m_layoutsController->inMultipleMode()) {
        if (selectedLayout.isActive && !m_layoutsController->selectedLayoutIsCurrentActive()) {
            m_corona->layoutsManager()->switchToLayout(selectedLayout.originalName());
        }
    } else {
        if (!m_layoutsController->selectedLayoutIsCurrentActive()) {
             m_corona->layoutsManager()->switchToLayout(selectedLayout.originalName());
        }
    }

    updatePerLayoutButtonsState();
}

void SettingsDialog::on_pauseButton_clicked()
{
    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayout();
    ui->pauseButton->setEnabled(false);

    if (m_layoutsController->inMultipleMode()
            && selectedLayout.isActive
            && !selectedLayout.isShared()
            && !m_layoutsController->selectedLayoutIsCurrentActive()) {
        m_corona->layoutsManager()->synchronizer()->pauseLayout(selectedLayout.originalName());
    }
}

void SettingsDialog::updateApplyButtonsState()
{
    bool changed{false};

    //! Ok, Apply Buttons
    if ((o_settingsOriginalData != currentSettings()) || (m_layoutsController->dataAreChanged())) {
        changed = true;
    }

    if (changed) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    } else {
        //ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    }

    //! RestoreDefaults Button
    if (ui->tabWidget->currentIndex() == 0) {
        ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
    } else if (ui->tabWidget->currentIndex() == 1) {
        //! Defaults for general Latte settings

        if (!ui->autostartChkBox->isChecked()
                || ui->badges3DStyleChkBox->isChecked()
                || ui->metaPressChkBox->isChecked()
                || !ui->metaPressHoldChkBox->isChecked()
                || !ui->infoWindowChkBox->isChecked()
                || ui->noBordersForMaximizedChkBox->isChecked()
                || !ui->highSensitivityBtn->isChecked()
                || ui->screenTrackerSpinBox->value() != SCREENTRACKERDEFAULTVALUE
                || ui->outlineSpinBox->value() != OUTLINEDEFAULTWIDTH ) {
            ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(true);
        } else {
            ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
        }
    }
}

void SettingsDialog::updatePerLayoutButtonsState()
{
    Settings::Data::Layout selectedLayout = m_layoutsController->selectedLayout();

    //! Switch Button
    if (selectedLayout.nameWasEdited()
            || (m_layoutsController->inMultipleMode() && selectedLayout.isShared())
            || (m_layoutsController->selectedLayoutIsCurrentActive())) {
        ui->switchButton->setEnabled(false);
    } else {
        ui->switchButton->setEnabled(true);
    }

    //! Pause Button
    if (!m_layoutsController->inMultipleMode()) {
        //! Single Layout mode
        ui->pauseButton->setVisible(false);
    } else {
        ui->pauseButton->setVisible(true);

        if (selectedLayout.isActive
                && !selectedLayout.isForFreeActivities()
                && !selectedLayout.isShared()) {
            ui->pauseButton->setEnabled(true);
        } else {
            ui->pauseButton->setEnabled(false);
        }
    }

    //! Remove Layout Button
    if (selectedLayout.isActive || selectedLayout.isLocked) {
        ui->removeButton->setEnabled(false);
    } else {
        ui->removeButton->setEnabled(true);
    }

    //! Layout Locked Button
    if (selectedLayout.isLocked) {
        ui->lockedButton->setChecked(true);
    } else {
        ui->lockedButton->setChecked(false);
    }

    //! Layout Shared Button
    if (selectedLayout.isShared()) {
        ui->sharedButton->setChecked(true);
    } else {
        ui->sharedButton->setChecked(false);
    }
}

void SettingsDialog::updateSharedLayoutsUiElements()
{
    //! UI Elements that need to be enabled/disabled
    if (m_layoutsController->inMultipleMode()) {
        ui->sharedButton->setVisible(true);
    } else {
        ui->sharedButton->setVisible(false);
    }
}

bool SettingsDialog::dataAreAccepted()
{
 /*   for (int i = 0; i < m_model->rowCount(); ++i) {
        QString layout1 = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        for (int j = i + 1; j < m_model->rowCount(); ++j) {
            QString temp = m_model->data(m_model->index(j, NAMECOLUMN), Qt::DisplayRole).toString();

            //!same layout name exists again
            if (layout1 == temp) {
                auto msg = new QMessageBox(this);
                msg->setIcon(QMessageBox::Warning);
                msg->setWindowTitle(i18n("Layout Warning"));
                msg->setText(i18n("There are layouts with the same name, that is not permitted!!! Please update these names to re-apply the changes..."));
                msg->setStandardButtons(QMessageBox::Ok);

                connect(msg, &QMessageBox::finished, this, [ &, i, j](int result) {
                    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect;
                    QModelIndex indexBase = m_model->index(i, NAMECOLUMN);
                    ui->layoutsView->selectionModel()->select(indexBase, flags);

                    QModelIndex indexOccurence = m_model->index(j, NAMECOLUMN);
                    ui->layoutsView->edit(indexOccurence);
                });


                msg->open();

                return false;
            }
        }
    }*/

    return true;
}

void SettingsDialog::showLayoutInformation()
{
  /*  int currentRow = ui->layoutsView->currentIndex().row();

    QString id = m_model->data(m_model->index(currentRow, IDCOLUMN), Qt::DisplayRole).toString();
    QString name = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::DisplayRole).toString();

    Layout::GenericLayout *genericActive= m_corona->layoutsManager()->synchronizer()->layout(o_layoutsOriginalData[id].originalName());
    Layout::GenericLayout *generic = genericActive ? genericActive : m_layouts[id];

    auto msg = new QMessageBox(this);
    msg->setWindowTitle(name);
    msg->setText(generic->reportHtml(m_corona->screenPool()));

    msg->open();*/
}

void SettingsDialog::showScreensInformation()
{
  /*  QList<int> assignedScreens;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        Layout::GenericLayout *genericActive= m_corona->layoutsManager()->synchronizer()->layout(o_layoutsOriginalData[id].originalName());
        Layout::GenericLayout *generic = genericActive ? genericActive : m_layouts[id];

        QList<int> vScreens = generic->viewsScreens();

        for (const int scrId : vScreens) {
            if (!assignedScreens.contains(scrId)) {
                assignedScreens << scrId;
            }
        }
    }

    auto msg = new QMessageBox(this);
    msg->setWindowTitle(i18n("Screens Information"));
    msg->setText(m_corona->screenPool()->reportHtml(assignedScreens));

    msg->open();*/
}

void SettingsDialog::saveAllChanges()
{
    if (!dataAreAccepted()) {
        return;
    }

    //! Update universal settings
    Latte::Types::MouseSensitivity sensitivity = static_cast<Latte::Types::MouseSensitivity>(m_mouseSensitivityButtons->checkedId());
    bool autostart = ui->autostartChkBox->isChecked();
    bool badges3DStyle = ui->badges3DStyleChkBox->isChecked();
    bool forwardMetaPress = ui->metaPressChkBox->isChecked();
    bool metaPressAndHold = ui->metaPressHoldChkBox->isChecked();
    bool showInfoWindow = ui->infoWindowChkBox->isChecked();
    bool noBordersForMaximized = ui->noBordersForMaximizedChkBox->isChecked();

    m_corona->universalSettings()->setMouseSensitivity(sensitivity);
    m_corona->universalSettings()->setAutostart(autostart);
    m_corona->universalSettings()->setBadges3DStyle(badges3DStyle);
    m_corona->universalSettings()->kwin_forwardMetaToLatte(forwardMetaPress);
    m_corona->universalSettings()->setMetaPressAndHoldEnabled(metaPressAndHold);
    m_corona->universalSettings()->setShowInfoWindow(showInfoWindow);
    m_corona->universalSettings()->setCanDisableBorders(noBordersForMaximized);
    m_corona->universalSettings()->setScreenTrackerInterval(ui->screenTrackerSpinBox->value());

    m_corona->themeExtended()->setOutlineWidth(ui->outlineSpinBox->value());

    o_settingsOriginalData = currentSettings();
    m_layoutsController->save();
}

}//end of namespace

