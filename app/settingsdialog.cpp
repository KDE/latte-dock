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

#include "dockcorona.h"
#include "layout.h"
#include "layoutmanager.h"
#include "importer.h"
#include "universalsettings.h"
#include "ui_settingsdialog.h"
#include "../liblattedock/dock.h"
#include "layoutsDelegates/checkboxdelegate.h"
#include "layoutsDelegates/colorcmbboxdelegate.h"
#include "layoutsDelegates/activitycmbboxdelegate.h"
#include "layoutsDelegates/layoutnamedelegate.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTemporaryDir>

#include <KActivities/Controller>
#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KLocalizedString>
#include <KNotification>
#include <KWindowSystem>
#include <KNewStuff3/KNS3/DownloadDialog>

namespace Latte {

const int IDCOLUMN = 0;
const int HIDDENTEXTCOLUMN = 1;
const int COLORCOLUMN = 2;
const int NAMECOLUMN = 3;
const int MENUCOLUMN = 4;
const int BORDERSCOLUMN = 5;
const int ACTIVITYCOLUMN = 6;

const int SCREENTRACKERDEFAULTVALUE = 2500;

const QChar CheckMark{0x2714};

SettingsDialog::SettingsDialog(QWidget *parent, DockCorona *corona)
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
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &SettingsDialog::restoreDefaults);

    m_model = new QStandardItemModel(m_corona->layoutManager()->layouts().count(), 6, this);

    ui->layoutsView->setModel(m_model);
    ui->layoutsView->horizontalHeader()->setStretchLastSection(true);
    ui->layoutsView->verticalHeader()->setVisible(false);

    connect(m_corona->layoutManager(), &LayoutManager::currentLayoutNameChanged, this, &SettingsDialog::layoutsChanged);
    connect(m_corona->layoutManager(), &LayoutManager::activeLayoutsChanged, this, &SettingsDialog::layoutsChanged);

    QString iconsPath(m_corona->kPackage().path() + "../../plasmoids/org.kde.latte.containment/contents/icons/");

    //!find the available colors
    QDir layoutDir(iconsPath);
    QStringList filter;
    filter.append(QString("*print.jpg"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);
    QStringList colors;

    foreach (auto file, files) {
        int colorEnd = file.lastIndexOf("print.jpg");
        QString color = file.remove(colorEnd, 9);
        colors.append(color);
    }

    ui->layoutsView->setItemDelegateForColumn(NAMECOLUMN, new LayoutNameDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(COLORCOLUMN, new ColorCmbBoxDelegate(this, iconsPath, colors));
    ui->layoutsView->setItemDelegateForColumn(MENUCOLUMN, new CheckBoxDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(BORDERSCOLUMN, new CheckBoxDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(ACTIVITYCOLUMN, new ActivityCmbBoxDelegate(this));

    m_inMemoryButtons = new QButtonGroup(this);
    m_inMemoryButtons->addButton(ui->singleToolBtn, Latte::Dock::SingleLayout);
    m_inMemoryButtons->addButton(ui->multipleToolBtn, Latte::Dock::MultipleLayouts);
    m_inMemoryButtons->setExclusive(true);

    if (KWindowSystem::isPlatformWayland()) {
        m_inMemoryButtons->button(Latte::Dock::MultipleLayouts)->setEnabled(false);
    }

    m_mouseSensitivityButtons = new QButtonGroup(this);
    m_mouseSensitivityButtons->addButton(ui->lowSensitivityBtn, Latte::Dock::LowSensitivity);
    m_mouseSensitivityButtons->addButton(ui->mediumSensitivityBtn, Latte::Dock::MediumSensitivity);
    m_mouseSensitivityButtons->addButton(ui->highSensitivityBtn, Latte::Dock::HighSensitivity);
    m_mouseSensitivityButtons->setExclusive(true);

    ui->screenTrackerSpinBox->setValue(m_corona->universalSettings()->screenTrackerInterval());

    //! About Menu
    QMenuBar *menuBar = new QMenuBar(this);
    // QMenuBar *rightAlignedMenuBar = new QMenuBar(menuBar);

    layout()->setMenuBar(menuBar);
    //menuBar->setCornerWidget(rightAlignedMenuBar);

    QMenu *fileMenu = new QMenu(i18n("File"), menuBar);
    menuBar->addMenu(fileMenu);

    QMenu *helpMenu = new QMenu(i18n("Help"), menuBar);
    //rightAlignedMenuBar->addMenu(helpMenu);
    menuBar->addMenu(helpMenu);

    QAction *quitAction = fileMenu->addAction(i18n("Quit Latte"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    QAction *aboutAction = helpMenu->addAction(i18n("About Latte"));
    aboutAction->setIcon(QIcon::fromTheme("latte-dock"));

    //! RTL support for labels in preferences
    if (qApp->layoutDirection() == Qt::RightToLeft) {
        ui->behaviorLbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
        ui->mouseSensetivityLbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
        ui->delayLbl->setAlignment(Qt::AlignRight | Qt::AlignTop);
    }

    loadSettings();

    //! SIGNALS

    connect(m_model, &QStandardItemModel::itemChanged, this, &SettingsDialog::itemChanged);
    connect(ui->layoutsView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [&]() {
        updatePerLayoutButtonsState();
        updateApplyButtonsState();
    });

    connect(m_inMemoryButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
    [ = ](int id, bool checked) {
        updateApplyButtonsState();
    });

    connect(m_mouseSensitivityButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
    [ = ](int id, bool checked) {
        updateApplyButtonsState();
    });

    connect(ui->screenTrackerSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        updateApplyButtonsState();
    });

    connect(ui->autostartChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->infoWindowChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsDialog::updateApplyButtonsState);

    connect(ui->noBordersForMaximizedChkBox, &QCheckBox::stateChanged, this, [&]() {
        bool noBordersForMaximized = ui->noBordersForMaximizedChkBox->isChecked();

        if (noBordersForMaximized) {
            ui->layoutsView->setColumnHidden(BORDERSCOLUMN, false);
        } else {
            ui->layoutsView->setColumnHidden(BORDERSCOLUMN, true);
        }

        updateApplyButtonsState();
    });

    connect(aboutAction, &QAction::triggered, m_corona, &DockCorona::aboutApplication);
    connect(quitAction, &QAction::triggered, m_corona, &DockCorona::closeApplication);

    //! update all layouts view when runningActivities changed. This way we update immediately
    //! the running Activities in Activities checkboxes which are shown as bold
    connect(m_corona->activitiesConsumer(), &KActivities::Consumer::runningActivitiesChanged,
    this, [&]() {
        ui->layoutsView->update();
    });

    blockDeleteOnActivityStopped();
}

SettingsDialog::~SettingsDialog()
{
    qDebug() << Q_FUNC_INFO;

    qDeleteAll(m_layouts);

    if (m_model) {
        delete m_model;
    }

    if (m_corona && m_corona->universalSettings()) {
        m_corona->universalSettings()->setLayoutsWindowSize(size());

        QStringList columnWidths;
        columnWidths << QString::number(ui->layoutsView->columnWidth(COLORCOLUMN));
        columnWidths << QString::number(ui->layoutsView->columnWidth(NAMECOLUMN));
        columnWidths << QString::number(ui->layoutsView->columnWidth(MENUCOLUMN));
        columnWidths << QString::number(ui->layoutsView->columnWidth(BORDERSCOLUMN));
        m_corona->universalSettings()->setLayoutsColumnWidths(columnWidths);
    }

    m_inMemoryButtons->deleteLater();
    m_mouseSensitivityButtons->deleteLater();

    foreach (auto tempDir, m_tempDirectories) {
        QDir tDir(tempDir);

        if (tDir.exists() && tempDir.startsWith("/tmp/")) {
            tDir.removeRecursively();
        }
    }
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

QStringList SettingsDialog::activities()
{
    return m_corona->layoutManager()->activities();
}

QStringList SettingsDialog::availableActivities()
{
    return m_availableActivities;
}

void SettingsDialog::setCurrentPage(Dock::LatteConfigPage page)
{
    if (page == Dock::LayoutPage) {
        ui->tabWidget->setCurrentIndex(0);
    } else if (page == Dock::PreferencesPage) {
        ui->tabWidget->setCurrentIndex(1);
    }
}

void SettingsDialog::on_newButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    //! find Default preset path
    foreach (auto preset, m_corona->layoutManager()->presetsPaths()) {
        QString presetName = Layout::layoutName(preset);

        if (presetName == "Default") {
            QByteArray presetNameChars = presetName.toUtf8();
            const char *prset_str = presetNameChars.data();
            presetName = uniqueLayoutName(i18n(prset_str));

            addLayoutForFile(preset, presetName, true, false);
            break;
        }
    }
}

void SettingsDialog::on_copyButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    //! Update original layout before copying if this layout is active
    if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
        QString lName = (m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole)).toString();

        if (Importer::layoutExists(lName)) {
            Layout *layout = m_corona->layoutManager()->activeLayout(lName);

            if (layout && layout->isOriginalLayout()) {
                layout->syncToLayoutFile();
            }
        }
    }

    QString tempDir = uniqueTempDirectory();

    QString id = m_model->data(m_model->index(row, IDCOLUMN), Qt::DisplayRole).toString();
    QString color = m_model->data(m_model->index(row, COLORCOLUMN), Qt::BackgroundRole).toString();
    QString textColor = m_model->data(m_model->index(row, COLORCOLUMN), Qt::UserRole).toString();
    QString layoutName = uniqueLayoutName(m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole).toString());
    bool menu = m_model->data(m_model->index(row, MENUCOLUMN), Qt::DisplayRole).toString() == CheckMark;
    bool disabledBorders = m_model->data(m_model->index(row, BORDERSCOLUMN), Qt::DisplayRole).toString() == CheckMark;

    QString copiedId = tempDir + "/" + layoutName + ".layout.latte";
    QFile(id).copy(copiedId);

    QFileInfo newFileInfo(copiedId);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(copiedId).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    Layout *settings = new Layout(this, copiedId);
    m_layouts[copiedId] = settings;

    insertLayoutInfoAtRow(row + 1, copiedId, color, textColor, layoutName, menu, disabledBorders, QStringList(), false);

    ui->layoutsView->selectRow(row + 1);
}

void SettingsDialog::on_downloadButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    KNS3::DownloadDialog dialog(QStringLiteral("latte-layouts.knsrc"), this);
    dialog.resize(m_corona->universalSettings()->downloadWindowSize());
    dialog.exec();

    bool layoutAdded{false};

    if (!dialog.changedEntries().isEmpty() || !dialog.installedEntries().isEmpty()) {
        foreach (auto entry, dialog.installedEntries()) {
            foreach (auto entryFile, entry.installedFiles()) {
                Importer::LatteFileVersion version = Importer::fileVersion(entryFile);

                if (version == Importer::LayoutVersion2) {
                    layoutAdded = true;
                    addLayoutForFile(entryFile);
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

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString layoutName = m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole).toString();

    if (m_corona->layoutManager()->activeLayout(layoutName)) {
        return;
    }

    m_model->removeRow(row);

    updateApplyButtonsState();

    row = qMax(row - 1, 0);

    ui->layoutsView->selectRow(row);
}

void SettingsDialog::on_lockedButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    bool lockedModel = m_model->data(m_model->index(row, NAMECOLUMN), Qt::UserRole).toBool();

    m_model->setData(m_model->index(row, NAMECOLUMN), QVariant(!lockedModel), Qt::UserRole);

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
        Importer::LatteFileVersion version = Importer::fileVersion(file);
        qDebug() << "VERSION :::: " << version;

        if (version == Importer::LayoutVersion2) {
            addLayoutForFile(file);
        } else if (version == Importer::ConfigVersion1) {
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
                importLayoutsFromV1ConfigFile(file);
            });

            connect(fullBtn, &QPushButton::clicked
            , this, [ &, file](bool check) {
                //!NOTE: Restart latte for import the new configuration
                QProcess::startDetached(qGuiApp->applicationFilePath() + " --import-full \"" + file + "\"");
                qGuiApp->exit();
            });
        } else if (version == Importer::ConfigVersion2) {
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

bool SettingsDialog::importLayoutsFromV1ConfigFile(QString file)
{
    KTar archive(file, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    //! if the file isnt a tar archive
    if (archive.isOpen()) {
        QDir tempDir{uniqueTempDirectory()};

        const auto archiveRootDir = archive.directory();

        foreach (auto &name, archiveRootDir->entries()) {
            auto fileEntry = archiveRootDir->file(name);
            fileEntry->copyTo(tempDir.absolutePath());
        }

        QString name = Importer::nameOfConfigFile(file);

        QString applets(tempDir.absolutePath() + "/" + "lattedock-appletsrc");

        if (QFile(applets).exists()) {
            if (m_corona->layoutManager()->importer()->importOldLayout(applets, name, false, tempDir.absolutePath())) {
                addLayoutForFile(tempDir.absolutePath() + "/" + name + ".layout.latte", name, false);
            }

            QString alternativeName = name + "-" + i18nc("layout", "Alternative");

            if (m_corona->layoutManager()->importer()->importOldLayout(applets, alternativeName, false, tempDir.absolutePath())) {
                addLayoutForFile(tempDir.absolutePath() + "/" + alternativeName + ".layout.latte", alternativeName, false);
            }
        }

        return true;
    }

    return false;
}


void SettingsDialog::on_exportButton_clicked()
{
    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString layoutExported = m_model->data(m_model->index(row, IDCOLUMN), Qt::DisplayRole).toString();

    //! Update ALL active original layouts before exporting,
    //! this is needed because the export method can export also the full configuration
    qDebug() << Q_FUNC_INFO;

    m_corona->layoutManager()->syncActiveLayoutsToOriginalFiles();

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

            Layout layoutS(this, file);
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

            if (m_corona->layoutManager()->importer()->exportFullConfiguration(file)) {

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

    QString background = m_model->data(m_model->index(row, COLORCOLUMN), Qt::BackgroundRole).toString();

    if (background.startsWith("/") && QFileInfo(background).exists()) {
        dialog.setDirectory(QFileInfo(background).absolutePath());
        dialog.selectFile(background);
    }

    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();

        if (files.count() > 0) {
            m_model->setData(m_model->index(row, COLORCOLUMN), files[0], Qt::BackgroundRole);
        }
    }
}

void SettingsDialog::requestColorsDialog(int row)
{
    QColorDialog dialog(this);
    QString textColor = m_model->data(m_model->index(row, COLORCOLUMN), Qt::UserRole).toString();
    dialog.setCurrentColor(QColor(textColor));

    if (dialog.exec()) {
        qDebug() << dialog.selectedColor().name();
        m_model->setData(m_model->index(row, COLORCOLUMN), dialog.selectedColor().name(), Qt::UserRole);
    }
}


void SettingsDialog::accept()
{
    qDebug() << Q_FUNC_INFO;

    if (saveAllChanges()) {
        deleteLater();
    }
}

void SettingsDialog::reject()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_blockDeleteOnReject) {
        deleteLater();
    }
}

void SettingsDialog::apply()
{
    qDebug() << Q_FUNC_INFO;
    saveAllChanges();

    o_settings = currentSettings();
    o_settingsLayouts = currentLayoutsSettings();

    updateApplyButtonsState();
    updatePerLayoutButtonsState();
}

void SettingsDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;

    if (ui->tabWidget->currentIndex() == 0) {
        //! Default layouts missing from layouts list
        foreach (auto preset, m_corona->layoutManager()->presetsPaths()) {
            QString presetName = Layout::layoutName(preset);
            QByteArray presetNameChars = presetName.toUtf8();
            const char *prset_str = presetNameChars.data();
            presetName = i18n(prset_str);

            if (!nameExistsInModel(presetName)) {
                addLayoutForFile(preset, presetName);
            }
        }
    } else if (ui->tabWidget->currentIndex() == 1) {
        //! Defaults for general Latte settings
        ui->autostartChkBox->setChecked(true);
        ui->infoWindowChkBox->setChecked(true);
        ui->noBordersForMaximizedChkBox->setChecked(false);
        ui->highSensitivityBtn->setChecked(true);
        ui->screenTrackerSpinBox->setValue(SCREENTRACKERDEFAULTVALUE);
    }
}

void SettingsDialog::addLayoutForFile(QString file, QString layoutName, bool newTempDirectory, bool showNotification)
{
    if (layoutName.isEmpty()) {
        layoutName = Layout::layoutName(file);
    }

    QString copiedId;

    if (newTempDirectory) {
        QString tempDir = uniqueTempDirectory();
        copiedId = tempDir + "/" + layoutName + ".layout.latte";
        QFile(file).copy(copiedId);
    } else {
        copiedId = file;
    }

    QFileInfo newFileInfo(copiedId);

    if (newFileInfo.exists() && !newFileInfo.isWritable()) {
        QFile(copiedId).setPermissions(QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    }

    if (m_layouts.contains(copiedId)) {
        Layout *oldSettings = m_layouts.take(copiedId);
        delete oldSettings;
    }

    Layout *settings = new Layout(this, copiedId);
    m_layouts[copiedId] = settings;

    QString id = copiedId;
    QString color = settings->color();
    QString textColor = settings->textColor();
    QString background = settings->background();
    bool menu = settings->showInMenu();
    bool disabledBorders = settings->disableBordersForMaximizedWindows();
    bool locked = !settings->isWritable();

    layoutName = uniqueLayoutName(layoutName);

    int row = ascendingRowFor(layoutName);

    if (background.isEmpty()) {
        insertLayoutInfoAtRow(row, copiedId, color, QString(), layoutName, menu, disabledBorders, QStringList(), locked);
    } else {
        insertLayoutInfoAtRow(row, copiedId, background, textColor, layoutName, menu, disabledBorders, QStringList(), locked);
    }

    ui->layoutsView->selectRow(row);

    if (showNotification) {
        //NOTE: The pointer is automatically deleted when the event is closed
        auto notification = new KNotification("import-done", KNotification::CloseOnTimeout);
        notification->setText(i18nc("import-done", "Layout: <b>%0</b> imported successfully<br>").arg(layoutName));
        notification->sendEvent();
    }
}

void SettingsDialog::loadSettings()
{
    m_initLayoutPaths.clear();
    m_model->clear();

    int i = 0;
    QStringList brokenLayouts;

    if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
        m_corona->layoutManager()->syncActiveLayoutsToOriginalFiles();
    }

    foreach (auto layout, m_corona->layoutManager()->layouts()) {
        QString layoutPath = QDir::homePath() + "/.config/latte/" + layout + ".layout.latte";
        m_initLayoutPaths.append(layoutPath);

        Layout *layoutSets = new Layout(this, layoutPath);
        m_layouts[layoutPath] = layoutSets;

        QString background = layoutSets->background();

        if (background.isEmpty()) {
            insertLayoutInfoAtRow(i, layoutPath, layoutSets->color(), QString(), layoutSets->name(),
                                  layoutSets->showInMenu(), layoutSets->disableBordersForMaximizedWindows(),
                                  layoutSets->activities(), !layoutSets->isWritable());
        } else {
            insertLayoutInfoAtRow(i, layoutPath, background, layoutSets->textColor(), layoutSets->name(),
                                  layoutSets->showInMenu(), layoutSets->disableBordersForMaximizedWindows(),
                                  layoutSets->activities(), !layoutSets->isWritable());
        }

        qDebug() << "counter:" << i << " total:" << m_model->rowCount();

        i++;

        if (layoutSets->name() == m_corona->layoutManager()->currentLayoutName()) {
            ui->layoutsView->selectRow(i - 1);
        }

        Layout *activeLayout = m_corona->layoutManager()->activeLayout(layoutSets->name());

        if ((activeLayout && activeLayout->layoutIsBroken()) || (!activeLayout && layoutSets->layoutIsBroken())) {
            brokenLayouts.append(layoutSets->name());
        }
    }


    //! Check Multiple Layouts Integrity
    /*if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
        m_corona->layoutManager()->syncActiveLayoutsToOriginalFiles();

        QString multipleLayoutPath = QDir::homePath() + "/.config/latte/" + Layout::MultipleLayoutsName + ".layout.latte";
        Layout multipleHiddenLayouts(this, multipleLayoutPath, "Multiple Layouts File");

        if (multipleHiddenLayouts.layoutIsBroken()) {
            qDebug() << "Intergrity Error ::: Multiple Layouts Hidden file is broken!!!!";
            brokenLayouts.append(multipleHiddenLayouts.name());
        }
    }*/


    recalculateAvailableActivities();

    m_model->setHorizontalHeaderItem(IDCOLUMN, new QStandardItem(QString("#path")));
    m_model->setHorizontalHeaderItem(COLORCOLUMN, new QStandardItem(QString(i18nc("column for layout background", "Background"))));
    m_model->setHorizontalHeaderItem(NAMECOLUMN, new QStandardItem(QString(i18nc("column for layout name", "Name"))));
    m_model->setHorizontalHeaderItem(MENUCOLUMN, new QStandardItem(QString(i18nc("column for layout to show in menu", "In Menu"))));
    m_model->setHorizontalHeaderItem(BORDERSCOLUMN, new QStandardItem(QString(i18nc("column for layout to hide borders for maximized windows", "Borderless"))));
    m_model->setHorizontalHeaderItem(ACTIVITYCOLUMN, new QStandardItem(QString(i18nc("column for layout to show which activities is assigned to", "Activities"))));

    //! this line should be commented for debugging layouts window functionality
    ui->layoutsView->setColumnHidden(IDCOLUMN, true);
    ui->layoutsView->setColumnHidden(HIDDENTEXTCOLUMN, true);

    if (m_corona->universalSettings()->canDisableBorders()) {
        ui->layoutsView->setColumnHidden(BORDERSCOLUMN, false);
    } else {
        ui->layoutsView->setColumnHidden(BORDERSCOLUMN, true);
    }

    ui->layoutsView->resizeColumnsToContents();

    QStringList columnWidths = m_corona->universalSettings()->layoutsColumnWidths();

    if (!columnWidths.isEmpty() && columnWidths.count() == 4) {
        ui->layoutsView->setColumnWidth(COLORCOLUMN, columnWidths[0].toInt());
        ui->layoutsView->setColumnWidth(NAMECOLUMN, columnWidths[1].toInt());
        ui->layoutsView->setColumnWidth(MENUCOLUMN, columnWidths[2].toInt());
        ui->layoutsView->setColumnWidth(BORDERSCOLUMN, columnWidths[3].toInt());
    }

    if (m_corona->layoutManager()->memoryUsage() == Dock::SingleLayout) {
        ui->singleToolBtn->setChecked(true);
    } else if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
        ui->multipleToolBtn->setChecked(true);
    }

    updatePerLayoutButtonsState();

    ui->autostartChkBox->setChecked(m_corona->universalSettings()->autostart());
    ui->infoWindowChkBox->setChecked(m_corona->universalSettings()->showInfoWindow());
    ui->noBordersForMaximizedChkBox->setChecked(m_corona->universalSettings()->canDisableBorders());

    if (m_corona->universalSettings()->mouseSensitivity() == Dock::LowSensitivity) {
        ui->lowSensitivityBtn->setChecked(true);
    } else if (m_corona->universalSettings()->mouseSensitivity() == Dock::MediumSensitivity) {
        ui->mediumSensitivityBtn->setChecked(true);
    } else if (m_corona->universalSettings()->mouseSensitivity() == Dock::HighSensitivity) {
        ui->highSensitivityBtn->setChecked(true);
    }

    o_settings = currentSettings();
    o_settingsLayouts = currentLayoutsSettings();
    updateApplyButtonsState();

    //! there are broken layouts and the user must be informed!
    if (brokenLayouts.count() > 0) {
        auto msg = new QMessageBox(this);
        msg->setIcon(QMessageBox::Warning);
        msg->setWindowTitle(i18n("Layout Warning"));
        msg->setText(i18n("The layout(s) <b>%0</b> have <i>broken configuration</i>!!! Please <b>remove them</b> to improve the system stability...").arg(brokenLayouts.join(",")));
        msg->setStandardButtons(QMessageBox::Ok);

        msg->open();
    }
}

QList<int> SettingsDialog::currentSettings()
{
    QList<int> settings;
    settings << m_inMemoryButtons->checkedId();
    settings << (int)ui->autostartChkBox->isChecked();
    settings << (int)ui->infoWindowChkBox->isChecked();
    settings << (int)ui->noBordersForMaximizedChkBox->isChecked();
    settings << m_mouseSensitivityButtons->checkedId();
    settings << ui->screenTrackerSpinBox->value();
    settings << m_model->rowCount();

    return settings;
}

QStringList SettingsDialog::currentLayoutsSettings()
{
    QStringList layoutSettings;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString color = m_model->data(m_model->index(i, COLORCOLUMN), Qt::BackgroundRole).toString();
        QString textColor = m_model->data(m_model->index(i, COLORCOLUMN), Qt::UserRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();
        bool locked = m_model->data(m_model->index(i, NAMECOLUMN), Qt::UserRole).toBool();
        bool menu = m_model->data(m_model->index(i, MENUCOLUMN), Qt::DisplayRole).toString() == CheckMark;
        bool borders = m_model->data(m_model->index(i, BORDERSCOLUMN), Qt::DisplayRole).toString() == CheckMark;
        QStringList lActivities = m_model->data(m_model->index(i, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        layoutSettings << id;
        layoutSettings << color;
        layoutSettings << textColor;
        layoutSettings << name;
        layoutSettings << QString::number((int)locked);
        layoutSettings << QString::number((int)menu);
        layoutSettings << QString::number((int)borders);
        layoutSettings << lActivities;
    }

    return layoutSettings;
}


void SettingsDialog::insertLayoutInfoAtRow(int row, QString path, QString color, QString textColor, QString name, bool menu,
        bool disabledBorders, QStringList activities, bool locked)
{
    QStandardItem *pathItem = new QStandardItem(path);

    QStandardItem *hiddenTextItem = new QStandardItem();

    QStandardItem *colorItem = new QStandardItem();
    colorItem->setSelectable(false);

    QStandardItem *nameItem = new QStandardItem(name);
    nameItem->setTextAlignment(Qt::AlignCenter);

    QStandardItem *menuItem = new QStandardItem();
    menuItem->setEditable(false);
    menuItem->setSelectable(true);
    menuItem->setText(menu ? CheckMark : QString());
    menuItem->setTextAlignment(Qt::AlignCenter);

    QStandardItem *bordersItem = new QStandardItem();
    bordersItem->setEditable(false);
    bordersItem->setSelectable(true);
    bordersItem->setText(disabledBorders ? CheckMark : QString());
    bordersItem->setTextAlignment(Qt::AlignCenter);

    QStandardItem *activitiesItem = new QStandardItem(activities.join(","));

    QList<QStandardItem *> items;

    items.append(pathItem);
    items.append(hiddenTextItem);
    items.append(colorItem);
    items.append(nameItem);
    items.append(menuItem);
    items.append(bordersItem);
    items.append(activitiesItem);

    if (row > m_model->rowCount() - 1) {
        m_model->appendRow(items);
        row = m_model->rowCount() - 1;

        qDebug() << "append row at:" << row << " rows:" << m_model->rowCount();
    } else {
        m_model->insertRow(row, items);
        qDebug() << "insert row at:" << row << " rows:" << m_model->rowCount();
    }

    m_model->setData(m_model->index(row, IDCOLUMN), path, Qt::DisplayRole);
    m_model->setData(m_model->index(row, COLORCOLUMN), color, Qt::BackgroundRole);
    m_model->setData(m_model->index(row, COLORCOLUMN), textColor, Qt::UserRole);

    QFont font;

    if (m_corona->layoutManager()->activeLayout(name)) {
        font.setBold(true);
    } else {
        font.setBold(false);
    }

    if (path.startsWith("/tmp/")) {
        font.setItalic(true);
    } else {
        font.setItalic(false);
    }

    m_model->setData(m_model->index(row, NAMECOLUMN), QVariant(name), Qt::DisplayRole);
    m_model->setData(m_model->index(row, NAMECOLUMN), font, Qt::FontRole);
    m_model->setData(m_model->index(row, NAMECOLUMN), QVariant(locked), Qt::UserRole);

    m_model->setData(m_model->index(row, ACTIVITYCOLUMN), activities, Qt::UserRole);
}


void SettingsDialog::on_switchButton_clicked()
{
    if (ui->buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
        //! thus there are changes in the settings

        QString lName;
        QStringList lActivities;

        if (m_inMemoryButtons->checkedId() == Latte::Dock::MultipleLayouts) {
            lName = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), NAMECOLUMN), Qt::DisplayRole).toString();
            lActivities = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), ACTIVITYCOLUMN), Qt::UserRole).toStringList();
        }

        apply();

        if (!lName.isEmpty() && !lActivities.isEmpty()) {
            //! an activities-assigned layout is chosen and at the same time we are moving
            //! to multiple layouts state
            m_corona->layoutManager()->switchToLayout(lName);
        }
    } else {
        QVariant value = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), NAMECOLUMN), Qt::DisplayRole);

        if (value.isValid()) {
            m_corona->layoutManager()->switchToLayout(value.toString());
        } else {
            qDebug() << "not valid layout";
        }
    }

    updatePerLayoutButtonsState();
}

void SettingsDialog::on_pauseButton_clicked()
{
    ui->pauseButton->setEnabled(false);

    QString id = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), IDCOLUMN), Qt::DisplayRole).toString();
    Layout *layout = m_layouts[id];

    if (layout) {
        m_corona->layoutManager()->pauseLayout(layout->name());
    }
}

void SettingsDialog::layoutsChanged()
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex nameIndex = m_model->index(i, NAMECOLUMN);
        QVariant value = m_model->data(nameIndex);

        if (value.isValid()) {
            QString name = value.toString();
            QFont font;

            if (m_corona->layoutManager()->currentLayoutName() == name) {
                font.setBold(true);
                // ui->layoutsView->selectRow(i);
            } else {
                Layout *layout = m_corona->layoutManager()->activeLayout(name);

                if (layout && (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts)) {
                    font.setBold(true);
                } else {
                    font.setBold(false);
                }
            }

            m_model->setData(nameIndex, font, Qt::FontRole);
        }
    }
}

void SettingsDialog::itemChanged(QStandardItem *item)
{
    updatePerLayoutButtonsState();

    if (item->column() == ACTIVITYCOLUMN) {
        //! recalculate the available activities
        recalculateAvailableActivities();
    } else if (item->column() == NAMECOLUMN) {
        int currentRow = ui->layoutsView->currentIndex().row();

        QString id = m_model->data(m_model->index(currentRow, IDCOLUMN), Qt::DisplayRole).toString();
        QString name = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::DisplayRole).toString();
        QFont font = qvariant_cast<QFont>(m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::FontRole));

        if (m_layouts[id]->name() != name) {
            font.setItalic(true);
            m_model->setData(m_model->index(currentRow, NAMECOLUMN), font, Qt::FontRole);
        } else {
            font.setItalic(false);
            m_model->setData(m_model->index(currentRow, NAMECOLUMN), font, Qt::FontRole);
        }
    }

    updateApplyButtonsState();
}

void SettingsDialog::updateApplyButtonsState()
{
    bool changed{false};

    //! Ok, Apply Buttons
    if ((o_settings != currentSettings())
        || (o_settingsLayouts != currentLayoutsSettings())) {
        changed = true;
    }

    if (changed) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    }

    //! RestoreDefaults Button
    if (ui->tabWidget->currentIndex() == 0) {
        //! Check Default layouts missing from layouts list

        bool layoutMissing{false};

        foreach (auto preset, m_corona->layoutManager()->presetsPaths()) {
            QString presetName = Layout::layoutName(preset);
            QByteArray presetNameChars = presetName.toUtf8();
            const char *prset_str = presetNameChars.data();
            presetName = i18n(prset_str);

            if (!nameExistsInModel(presetName)) {
                layoutMissing = true;
                break;
            }
        }

        if (layoutMissing) {
            ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(true);
        } else {
            ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
        }
    } else if (ui->tabWidget->currentIndex() == 1) {
        //! Defaults for general Latte settings

        if (!ui->autostartChkBox->isChecked() || !ui->infoWindowChkBox->isChecked() || ui->noBordersForMaximizedChkBox->isChecked()
            || !ui->highSensitivityBtn->isChecked() || ui->screenTrackerSpinBox->value() != SCREENTRACKERDEFAULTVALUE) {
            ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(true);
        } else {
            ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
        }
    }
}

void SettingsDialog::updatePerLayoutButtonsState()
{
    int currentRow = ui->layoutsView->currentIndex().row();

    QString id = m_model->data(m_model->index(currentRow, IDCOLUMN), Qt::DisplayRole).toString();
    QString nameInModel = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::DisplayRole).toString();
    QString originalName = m_layouts.contains(id) ? m_layouts[id]->name() : "";
    bool lockedInModel = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::UserRole).toBool();

    //! Switch Button
    if (id.startsWith("/tmp/") || originalName != nameInModel) {
        ui->switchButton->setEnabled(false);
    } else {
        ui->switchButton->setEnabled(true);
    }

    //! Pause Button
    if (m_corona->layoutManager()->memoryUsage() == Dock::SingleLayout) {
        ui->pauseButton->setVisible(false);
    } else if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
        ui->pauseButton->setVisible(true);

        QStringList lActivities = m_model->data(m_model->index(currentRow, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        Layout *layout = m_layouts[id];

        if (!lActivities.isEmpty() && layout && m_corona->layoutManager()->activeLayout(layout->name())) {
            ui->pauseButton->setEnabled(true);
        } else {
            ui->pauseButton->setEnabled(false);
        }
    }

    //! Remove Layout Button
    if (originalName != nameInModel
        || (originalName == m_corona->layoutManager()->currentLayoutName())
        || (m_corona->layoutManager()->activeLayout(originalName))
        || lockedInModel) {
        ui->removeButton->setEnabled(false);
    } else {
        ui->removeButton->setEnabled(true);
    }

    if (lockedInModel) {
        ui->lockedButton->setChecked(true);
    } else {
        ui->lockedButton->setChecked(false);
    }
}

void SettingsDialog::recalculateAvailableActivities()
{
    QStringList tempActivities = m_corona->layoutManager()->activities();

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStringList assigned = m_model->data(m_model->index(i, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        foreach (auto activity, assigned) {
            if (tempActivities.contains(activity)) {
                tempActivities.removeAll(activity);
            }
        }
    }

    m_availableActivities = tempActivities;
}

bool SettingsDialog::dataAreAccepted()
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
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
    }

    return true;
}

bool SettingsDialog::saveAllChanges()
{
    if (!dataAreAccepted()) {
        return false;
    }

    //! Update universal settings
    Latte::Dock::MouseSensitivity sensitivity = static_cast<Latte::Dock::MouseSensitivity>(m_mouseSensitivityButtons->checkedId());
    bool autostart = ui->autostartChkBox->isChecked();
    bool showInfoWindow = ui->infoWindowChkBox->isChecked();
    bool noBordersForMaximized = ui->noBordersForMaximizedChkBox->isChecked();

    m_corona->universalSettings()->setMouseSensitivity(sensitivity);
    m_corona->universalSettings()->setAutostart(autostart);
    m_corona->universalSettings()->setShowInfoWindow(showInfoWindow);
    m_corona->universalSettings()->setCanDisableBorders(noBordersForMaximized);
    m_corona->universalSettings()->setScreenTrackerInterval(ui->screenTrackerSpinBox->value());

    //! Update Layouts
    QStringList knownActivities = activities();

    QTemporaryDir layoutTempDir;

    qDebug() << "Temporary Directory ::: " << layoutTempDir.path();

    QStringList fromRenamePaths;
    QStringList toRenamePaths;
    QStringList toRenameNames;

    QString switchToLayout;

    QHash<QString, Layout *> activeLayoutsToRename;

    //! remove layouts that have been removed from the user
    foreach (auto initLayout, m_initLayoutPaths) {
        if (!idExistsInModel(initLayout)) {
            QFile(initLayout).remove();

            if (m_layouts.contains(initLayout)) {
                Layout *removedLayout = m_layouts.take(initLayout);
                delete removedLayout;
            }
        }
    }

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString color = m_model->data(m_model->index(i, COLORCOLUMN), Qt::BackgroundRole).toString();
        QString textColor = m_model->data(m_model->index(i, COLORCOLUMN), Qt::UserRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();
        bool locked = m_model->data(m_model->index(i, NAMECOLUMN), Qt::UserRole).toBool();
        bool menu = m_model->data(m_model->index(i, MENUCOLUMN), Qt::DisplayRole).toString() == CheckMark;
        bool disabledBorders = m_model->data(m_model->index(i, BORDERSCOLUMN), Qt::DisplayRole).toString() == CheckMark;
        QStringList lActivities = m_model->data(m_model->index(i, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        QStringList cleanedActivities;

        //!update only activities that are valid
        foreach (auto activity, lActivities) {
            if (knownActivities.contains(activity)) {
                cleanedActivities.append(activity);
            }
        }

        //qDebug() << i << ". " << id << " - " << color << " - " << name << " - " << menu << " - " << lActivities;
        Layout *activeLayout = m_corona->layoutManager()->activeLayout(m_layouts[id]->name());

        Layout *layout = activeLayout ? activeLayout : m_layouts[id];

        //! unlock read-only layout
        if (!layout->isWritable()) {
            layout->unlock();
        }

        if (color.startsWith("/")) {
            //it is image file in such case
            if (color != layout->background()) {
                layout->setBackground(color);
            }

            if (layout->textColor() != textColor) {
                layout->setTextColor(textColor);
            }
        } else {
            if (color != layout->color()) {
                layout->setColor(color);
                layout->setBackground(QString());
                layout->setTextColor(QString());
            }
        }

        if (layout->showInMenu() != menu) {
            layout->setShowInMenu(menu);
        }

        if (layout->disableBordersForMaximizedWindows() != disabledBorders) {
            layout->setDisableBordersForMaximizedWindows(disabledBorders);
        }

        if (layout->activities() != cleanedActivities) {
            layout->setActivities(cleanedActivities);
        }

        //! If the layout name changed OR the layout path is a temporary one
        if (layout->name() != name || (id.startsWith("/tmp/"))) {
            //! If the layout is Active in MultipleLayouts
            if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts && activeLayout) {
                qDebug() << " Active Layout Should Be Renamed From : " << layout->name() << " TO :: " << name;
                activeLayoutsToRename[name] = layout;
            }

            QString tempFile = layoutTempDir.path() + "/" + QString(layout->name() + ".layout.latte");
            qDebug() << "new temp file ::: " << tempFile;

            if ((m_corona->layoutManager()->memoryUsage() == Dock::SingleLayout) && (layout->name() == m_corona->layoutManager()->currentLayoutName())) {
                switchToLayout = name;
            }

            layout = m_layouts.take(id);
            delete layout;

            QFile(id).rename(tempFile);

            fromRenamePaths.append(id);
            toRenamePaths.append(tempFile);
            toRenameNames.append(name);
        }
    }

    //! this is necessary in case two layouts have to swap names
    //! so we copy first the layouts in a temp directory and afterwards all
    //! together we move them in the official layout directory
    for (int i = 0; i < toRenamePaths.count(); ++i) {
        QString newFile = QDir::homePath() + "/.config/latte/" + toRenameNames[i] + ".layout.latte";
        QFile(toRenamePaths[i]).rename(newFile);

        Layout *nLayout = new Layout(this, newFile);
        m_layouts[newFile] = nLayout;

        for (int j = 0; j < m_model->rowCount(); ++j) {
            QString tId = m_model->data(m_model->index(j, IDCOLUMN), Qt::DisplayRole).toString();

            if (tId == fromRenamePaths[i]) {
                m_model->setData(m_model->index(j, IDCOLUMN), newFile, Qt::DisplayRole);
                m_initLayoutPaths.append(newFile);

                QFont font = qvariant_cast<QFont>(m_model->data(m_model->index(j, NAMECOLUMN), Qt::FontRole));

                font.setItalic(false);
                m_model->setData(m_model->index(j, NAMECOLUMN), font, Qt::FontRole);
            }
        }
    }

    QString orphanedLayout;

    if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
        foreach (auto newLayoutName, activeLayoutsToRename.keys()) {
            qDebug() << " Active Layout Is Renamed From : " << activeLayoutsToRename[newLayoutName]->name() << " TO :: " << newLayoutName;
            Layout *layout = activeLayoutsToRename[newLayoutName];
            layout->renameLayout(newLayoutName);

            //! that means it is an active layout for orphaned Activities
            if (layout->activities().isEmpty()) {
                orphanedLayout = newLayoutName;
            }
        }
    }

    //! lock layouts in the end when the user has chosen it
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();
        bool locked = m_model->data(m_model->index(i, NAMECOLUMN), Qt::UserRole).toBool();

        Layout *activeLayout = m_corona->layoutManager()->activeLayout(m_layouts[id]->name());
        Layout *layout = activeLayout ? activeLayout : m_layouts[id];

        if (layout && locked && layout->isWritable()) {
            layout->lock();
        }
    }

    m_corona->layoutManager()->loadLayouts();

    Latte::Dock::LayoutsMemoryUsage inMemoryOption = static_cast<Latte::Dock::LayoutsMemoryUsage>(m_inMemoryButtons->checkedId());

    if (m_corona->layoutManager()->memoryUsage() != inMemoryOption) {
        Dock::LayoutsMemoryUsage previousMemoryUsage = m_corona->layoutManager()->memoryUsage();
        m_corona->layoutManager()->setMemoryUsage(inMemoryOption);

        QVariant value = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), NAMECOLUMN), Qt::DisplayRole);
        QString layoutName = value.toString();

        m_corona->layoutManager()->switchToLayout(layoutName, previousMemoryUsage);
    } else {
        if (!switchToLayout.isEmpty()) {
            m_corona->layoutManager()->switchToLayout(switchToLayout);
        } else if (m_corona->layoutManager()->memoryUsage() == Dock::MultipleLayouts) {
            m_corona->layoutManager()->syncMultipleLayoutsToActivities(orphanedLayout);
        }
    }

    return true;
}

bool SettingsDialog::idExistsInModel(QString id)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowId = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();

        if (rowId == id) {
            return true;
        }
    }

    return false;
}

bool SettingsDialog::nameExistsInModel(QString name)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowName = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        if (rowName == name) {
            return true;
        }
    }

    return false;
}

int SettingsDialog::ascendingRowFor(QString name)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowName = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        if (rowName.toUpper() > name.toUpper()) {
            return i;
        }
    }

    return m_model->rowCount();
}

QString SettingsDialog::uniqueTempDirectory()
{
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    m_tempDirectories.append(tempDir.path());

    return tempDir.path();
}

QString SettingsDialog::uniqueLayoutName(QString name)
{
    int pos_ = name.lastIndexOf(QRegExp(QString("[-][0-9]+")));

    if (nameExistsInModel(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (nameExistsInModel(name)) {
        name = namePart + "-" + QString::number(i);
        i++;
    }

    return name;
}

}//end of namespace

