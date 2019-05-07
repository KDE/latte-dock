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
#include "layoutmanager.h"
#include "importer.h"
#include "universalsettings.h"
#include "ui_settingsdialog.h"
#include "../lattecorona.h"
#include "../layout/genericlayout.h"
#include "../layout/centrallayout.h"
#include "../layout/sharedlayout.h"
#include "../liblatte2/types.h"
#include "../plasma/extended/theme.h"
#include "delegates/activitiesdelegate.h"
#include "delegates/checkboxdelegate.h"
#include "delegates/colorcmbboxdelegate.h"
#include "delegates/layoutnamedelegate.h"
#include "delegates/shareddelegate.h"

// Qt
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

// KDE
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
const int SHAREDCOLUMN = 7;

const int SCREENTRACKERDEFAULTVALUE = 2500;
const int OUTLINEDEFAULTWIDTH = 1;

const QChar CheckMark{0x2714};

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
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &SettingsDialog::restoreDefaults);

    m_model = new QStandardItemModel(m_corona->layoutManager()->layouts().count(), 6, this);

    ui->layoutsView->setModel(m_model);
    ui->layoutsView->horizontalHeader()->setStretchLastSection(true);
    ui->layoutsView->verticalHeader()->setVisible(false);

    connect(m_corona->layoutManager(), &LayoutManager::currentLayoutNameChanged, this, &SettingsDialog::layoutsChanged);
    connect(m_corona->layoutManager(), &LayoutManager::centralLayoutsChanged, this, &SettingsDialog::layoutsChanged);

    QString iconsPath(m_corona->kPackage().path() + "../../plasmoids/org.kde.latte.containment/contents/icons/");

    //!find the available colors
    QDir layoutDir(iconsPath);
    QStringList filter;
    filter.append(QString("*print.jpg"));
    QStringList files = layoutDir.entryList(filter, QDir::Files | QDir::NoSymLinks);
    QStringList colors;

    for (auto &file : files) {
        int colorEnd = file.lastIndexOf("print.jpg");
        QString color = file.remove(colorEnd, 9);
        colors.append(color);
    }

    ui->layoutsView->setItemDelegateForColumn(NAMECOLUMN, new LayoutNameDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(COLORCOLUMN, new ColorCmbBoxDelegate(this, iconsPath, colors));
    ui->layoutsView->setItemDelegateForColumn(MENUCOLUMN, new CheckBoxDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(BORDERSCOLUMN, new CheckBoxDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(ACTIVITYCOLUMN, new ActivitiesDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(SHAREDCOLUMN, new SharedDelegate(this));

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

    connect(ui->outlineSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [ = ](int i) {
        updateApplyButtonsState();
    });

    connect(ui->autostartChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->metaPressChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(ui->metaPressHoldChkBox, &QCheckBox::stateChanged, this, &SettingsDialog::updateApplyButtonsState);
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

    connect(aboutAction, &QAction::triggered, m_corona, &Latte::Corona::aboutApplication);
    connect(quitAction, &QAction::triggered, m_corona, &Latte::Corona::closeApplication);

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

    for (const auto &tempDir : m_tempDirectories) {
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

QStringList SettingsDialog::availableSharesFor(int row)
{
    QStringList availables;
    QStringList regs;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QStringList shares = m_model->data(m_model->index(i, SHAREDCOLUMN), Qt::UserRole).toStringList();

        if (i != row) {
            if (shares.isEmpty()) {
                availables << id;
            } else {
                regs << shares;
            }
        }
    }

    for (const auto r : regs) {
        availables.removeAll(r);
    }

    return availables;
}

void SettingsDialog::setCurrentPage(Types::LatteConfigPage page)
{
    if (page == Types::LayoutPage) {
        ui->tabWidget->setCurrentIndex(0);
    } else if (page == Types::PreferencesPage) {
        ui->tabWidget->setCurrentIndex(1);
    }
}

void SettingsDialog::on_newButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    //! find Default preset path
    for (const auto &preset : m_corona->layoutManager()->presetsPaths()) {
        QString presetName = CentralLayout::layoutName(preset);

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
    if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        QString lName = (m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole)).toString();

        Layout::GenericLayout *generic = m_corona->layoutManager()->layout(lName);
        if (generic) {
            generic->syncToLayoutFile();
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

    CentralLayout *settings = new CentralLayout(this, copiedId);
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
        for (const auto &entry : dialog.installedEntries()) {
            for (const auto &entryFile : entry.installedFiles()) {
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

    if (m_corona->layoutManager()->centralLayout(layoutName)) {
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

        for (const auto &name : archiveRootDir->entries()) {
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
        for (const auto &preset : m_corona->layoutManager()->presetsPaths()) {
            QString presetName = CentralLayout::layoutName(preset);
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
        ui->metaPressChkBox->setChecked(false);
        ui->metaPressHoldChkBox->setChecked(true);
        ui->noBordersForMaximizedChkBox->setChecked(false);
        ui->highSensitivityBtn->setChecked(true);
        ui->screenTrackerSpinBox->setValue(SCREENTRACKERDEFAULTVALUE);
        ui->outlineSpinBox->setValue(OUTLINEDEFAULTWIDTH);
    }
}

void SettingsDialog::addLayoutForFile(QString file, QString layoutName, bool newTempDirectory, bool showNotification)
{
    if (layoutName.isEmpty()) {
        layoutName = CentralLayout::layoutName(file);
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
        CentralLayout *oldSettings = m_layouts.take(copiedId);
        delete oldSettings;
    }

    CentralLayout *settings = new CentralLayout(this, copiedId);
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
    m_sharesMap.clear();

    int i = 0;
    QStringList brokenLayouts;

    if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        m_corona->layoutManager()->syncActiveLayoutsToOriginalFiles();
    }

    for (const auto layout : m_corona->layoutManager()->layouts()) {
        QString layoutPath = QDir::homePath() + "/.config/latte/" + layout + ".layout.latte";
        m_initLayoutPaths.append(layoutPath);

        CentralLayout *central = new CentralLayout(this, layoutPath);
        m_layouts[layoutPath] = central;

        QString background = central->background();

        //! add central layout properties
        if (background.isEmpty()) {
            insertLayoutInfoAtRow(i, layoutPath, central->color(), QString(), central->name(),
                                  central->showInMenu(), central->disableBordersForMaximizedWindows(),
                                  central->activities(), !central->isWritable());
        } else {
            insertLayoutInfoAtRow(i, layoutPath, background, central->textColor(), central->name(),
                                  central->showInMenu(), central->disableBordersForMaximizedWindows(),
                                  central->activities(), !central->isWritable());
        }

        //! create initial SHARES maps
        QString shared = central->sharedLayoutName();
        if (!shared.isEmpty()) {
            m_sharesMap[shared].append(layoutPath);
        }

        qDebug() << "counter:" << i << " total:" << m_model->rowCount();

        i++;

        if (central->name() == m_corona->layoutManager()->currentLayoutName()) {
            ui->layoutsView->selectRow(i - 1);
        }

        Layout::GenericLayout *generic = m_corona->layoutManager()->layout(central->name());

        if ((generic && generic->layoutIsBroken()) || (!generic && central->layoutIsBroken())) {
            brokenLayouts.append(central->name());
        }
    }

    //! update SHARES map keys in order to use the #settingsid(s)
    QStringList forremoval;

    //! remove these records after updating
    for (QHash<const QString, QStringList>::iterator i=m_sharesMap.begin(); i!=m_sharesMap.end(); ++i) {
        forremoval << i.key();
    }

    //! update keys
    for (QHash<const QString, QStringList>::iterator i=m_sharesMap.begin(); i!=m_sharesMap.end(); ++i) {
        QString shareid = idForRow(rowForName(i.key()));
        m_sharesMap[shareid] = i.value();
    }

    //! remove deprecated keys
    for (const auto &key : forremoval) {
        m_sharesMap.remove(key);
    }

    qDebug() << "SHARES MAP ::: " << m_sharesMap;

    for (QHash<const QString, QStringList>::iterator i=m_sharesMap.begin(); i!=m_sharesMap.end(); ++i) {
        int sharedPos = rowForId(i.key());

        if (sharedPos >= 0) {
            m_model->setData(m_model->index(sharedPos, SHAREDCOLUMN), i.value(), Qt::UserRole);
        }
    }

    recalculateAvailableActivities();

    m_model->setHorizontalHeaderItem(IDCOLUMN, new QStandardItem(QString("#path")));
    m_model->setHorizontalHeaderItem(COLORCOLUMN, new QStandardItem(QIcon::fromTheme("games-config-background"),
                                                                    QString(i18nc("column for layout background", "Background"))));
    m_model->setHorizontalHeaderItem(NAMECOLUMN, new QStandardItem(QString(i18nc("column for layout name", "Name"))));
    m_model->setHorizontalHeaderItem(MENUCOLUMN, new QStandardItem(QString(i18nc("column for layout to show in menu", "In Menu"))));
    m_model->setHorizontalHeaderItem(BORDERSCOLUMN, new QStandardItem(QString(i18nc("column for layout to hide borders for maximized windows", "Borderless"))));
    m_model->setHorizontalHeaderItem(ACTIVITYCOLUMN, new QStandardItem(QIcon::fromTheme("preferences-activities"),
                                                                       QString(i18nc("column for layout to show which activities is assigned to", "Activities"))));
    m_model->setHorizontalHeaderItem(SHAREDCOLUMN, new QStandardItem(QIcon::fromTheme("document-share"),
                                                                     QString(i18nc("column for shared layout to show which layouts is assigned to", "Shared To"))));

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

    if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout) {
        ui->singleToolBtn->setChecked(true);
    } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        ui->multipleToolBtn->setChecked(true);
    }

    updatePerLayoutButtonsState();
    updateSharedLayoutsStates();

    ui->autostartChkBox->setChecked(m_corona->universalSettings()->autostart());
    ui->infoWindowChkBox->setChecked(m_corona->universalSettings()->showInfoWindow());
    ui->metaPressChkBox->setChecked(m_corona->universalSettings()->metaForwardedToLatte());
    ui->metaPressHoldChkBox->setChecked(m_corona->universalSettings()->metaPressAndHoldEnabled());
    ui->noBordersForMaximizedChkBox->setChecked(m_corona->universalSettings()->canDisableBorders());

    if (m_corona->universalSettings()->mouseSensitivity() == Types::LowSensitivity) {
        ui->lowSensitivityBtn->setChecked(true);
    } else if (m_corona->universalSettings()->mouseSensitivity() == Types::MediumSensitivity) {
        ui->mediumSensitivityBtn->setChecked(true);
    } else if (m_corona->universalSettings()->mouseSensitivity() == Types::HighSensitivity) {
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
    settings << (int)ui->metaPressChkBox->isChecked();
    settings << (int)ui->metaPressHoldChkBox->isChecked();
    settings << (int)ui->noBordersForMaximizedChkBox->isChecked();
    settings << m_mouseSensitivityButtons->checkedId();
    settings << ui->screenTrackerSpinBox->value();
    settings << ui->outlineSpinBox->value();
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
        QStringList shares = m_model->data(m_model->index(i, SHAREDCOLUMN), Qt::UserRole).toStringList();

        layoutSettings << id;
        layoutSettings << color;
        layoutSettings << textColor;
        layoutSettings << name;
        layoutSettings << QString::number((int)locked);
        layoutSettings << QString::number((int)menu);
        layoutSettings << QString::number((int)borders);
        layoutSettings << lActivities;
        layoutSettings << shares;
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

    QStandardItem *sharesItem = new QStandardItem();

    QList<QStandardItem *> items;

    items.append(pathItem);
    items.append(hiddenTextItem);
    items.append(colorItem);
    items.append(nameItem);
    items.append(menuItem);
    items.append(bordersItem);
    items.append(activitiesItem);
    items.append(sharesItem);

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

    if (m_corona->layoutManager()->layout(name)) {
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

        if (m_inMemoryButtons->checkedId() == Latte::Types::MultipleLayouts) {
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
    CentralLayout *layout = m_layouts[id];

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
                Layout::GenericLayout *layout = m_corona->layoutManager()->layout(name);

                if (layout && (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts)) {
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

        if (m_corona->layoutManager()->layout(m_layouts[id]->name())) {
            font.setBold(true);
        } else {
            font.setBold(false);
        }

        if (m_layouts[id]->name() != name) {
            font.setItalic(true);
        } else {
            font.setItalic(false);
        }

        m_model->setData(m_model->index(currentRow, NAMECOLUMN), font, Qt::FontRole);

    } else if (item->column() == SHAREDCOLUMN) {
        updateSharedLayoutsStates();
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
        //ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    }

    //! RestoreDefaults Button
    if (ui->tabWidget->currentIndex() == 0) {
        //! Check Default layouts missing from layouts list

        bool layoutMissing{false};

        for (const auto &preset : m_corona->layoutManager()->presetsPaths()) {
            QString presetName = CentralLayout::layoutName(preset);
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

        if (!ui->autostartChkBox->isChecked()
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
    int currentRow = ui->layoutsView->currentIndex().row();

    QString id = m_model->data(m_model->index(currentRow, IDCOLUMN), Qt::DisplayRole).toString();
    QString nameInModel = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::DisplayRole).toString();
    QString originalName = m_layouts.contains(id) ? m_layouts[id]->name() : "";
    bool lockedInModel = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::UserRole).toBool();
    bool sharedInModel = !m_model->data(m_model->index(currentRow, SHAREDCOLUMN), Qt::UserRole).toStringList().isEmpty();

    //! Switch Button
    if (id.startsWith("/tmp/") || originalName != nameInModel) {
        ui->switchButton->setEnabled(false);
    } else {
        ui->switchButton->setEnabled(true);
    }

    //! Pause Button
    if (m_corona->layoutManager()->memoryUsage() == Types::SingleLayout) {
        ui->pauseButton->setVisible(false);
    } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        ui->pauseButton->setVisible(true);

        QStringList lActivities = m_model->data(m_model->index(currentRow, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        Latte::CentralLayout *layout = m_layouts[id];

        if (!lActivities.isEmpty() && layout && m_corona->layoutManager()->centralLayout(layout->name())) {
            ui->pauseButton->setEnabled(true);
        } else {
            ui->pauseButton->setEnabled(false);
        }
    }

    //! Remove Layout Button
    if (originalName != nameInModel
            || (originalName == m_corona->layoutManager()->currentLayoutName())
            || (m_corona->layoutManager()->centralLayout(originalName))
            || lockedInModel) {
        ui->removeButton->setEnabled(false);
    } else {
        ui->removeButton->setEnabled(true);
    }

    //! Layout Locked Button
    if (lockedInModel) {
        ui->lockedButton->setChecked(true);
    } else {
        ui->lockedButton->setChecked(false);
    }

    //! Layout Shared Button
    if (sharedInModel) {
        ui->sharedButton->setChecked(true);
    } else {
        ui->sharedButton->setChecked(false);
    }
}

void SettingsDialog::updateSharedLayoutsStates()
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStringList shares = m_model->data(m_model->index(i, SHAREDCOLUMN), Qt::UserRole).toStringList();

        if (shares.isEmpty()) {
            QStandardItem *item = m_model->item(i, MENUCOLUMN);
            item->setEnabled(true);
            item = m_model->item(i, BORDERSCOLUMN);
            item->setEnabled(true);
            item = m_model->item(i,ACTIVITYCOLUMN);
            item->setEnabled(true);
        } else {
            QStandardItem *item = m_model->item(i, MENUCOLUMN);
            item->setEnabled(false);
            item = m_model->item(i, BORDERSCOLUMN);
            item->setEnabled(false);
            item = m_model->item(i,ACTIVITYCOLUMN);
            item->setEnabled(false);
        }
    }
}

void SettingsDialog::recalculateAvailableActivities()
{
    QStringList tempActivities = m_corona->layoutManager()->activities();

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStringList assigned = m_model->data(m_model->index(i, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        for (const auto &activity : assigned) {
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
    Latte::Types::MouseSensitivity sensitivity = static_cast<Latte::Types::MouseSensitivity>(m_mouseSensitivityButtons->checkedId());
    bool autostart = ui->autostartChkBox->isChecked();
    bool forwardMetaPress = ui->metaPressChkBox->isChecked();
    bool metaPressAndHold = ui->metaPressHoldChkBox->isChecked();
    bool showInfoWindow = ui->infoWindowChkBox->isChecked();
    bool noBordersForMaximized = ui->noBordersForMaximizedChkBox->isChecked();

    m_corona->universalSettings()->setMouseSensitivity(sensitivity);
    m_corona->universalSettings()->setAutostart(autostart);
    m_corona->universalSettings()->forwardMetaToLatte(forwardMetaPress);
    m_corona->universalSettings()->setMetaPressAndHoldEnabled(metaPressAndHold);
    m_corona->universalSettings()->setShowInfoWindow(showInfoWindow);
    m_corona->universalSettings()->setCanDisableBorders(noBordersForMaximized);
    m_corona->universalSettings()->setScreenTrackerInterval(ui->screenTrackerSpinBox->value());

    m_corona->themeExtended()->setOutlineWidth(ui->outlineSpinBox->value());

    //! Update Layouts
    QStringList knownActivities = activities();

    QTemporaryDir layoutTempDir;

    qDebug() << "Temporary Directory ::: " << layoutTempDir.path();

    QStringList fromRenamePaths;
    QStringList toRenamePaths;
    QStringList toRenameNames;

    QString switchToLayout;

    QHash<QString, Layout::GenericLayout *> activeLayoutsToRename;

    //! remove layouts that have been removed from the user
    for (const auto &initLayout : m_initLayoutPaths) {
        if (!idExistsInModel(initLayout)) {
            QFile(initLayout).remove();

            if (m_layouts.contains(initLayout)) {
                CentralLayout *removedLayout = m_layouts.take(initLayout);
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
        for (const auto &activity : lActivities) {
            if (knownActivities.contains(activity)) {
                cleanedActivities.append(activity);
            }
        }

        //qDebug() << i << ". " << id << " - " << color << " - " << name << " - " << menu << " - " << lActivities;
        //! update the generic parts of the layouts
        Layout::GenericLayout *genericActive= m_corona->layoutManager()->layout(m_layouts[id]->name());
        Layout::GenericLayout *generic = genericActive ? genericActive : m_layouts[id];

        //! unlock read-only layout
        if (!generic->isWritable()) {
            generic->unlock();
        }

        if (color.startsWith("/")) {
            //it is image file in such case
            if (color != generic->background()) {
                generic->setBackground(color);
            }

            if (generic->textColor() != textColor) {
                generic->setTextColor(textColor);
            }
        } else {
            if (color != generic->color()) {
                generic->setColor(color);
                generic->setBackground(QString());
                generic->setTextColor(QString());
            }
        }

        //! update only the Central-specific layout parts
        CentralLayout *centralActive= m_corona->layoutManager()->centralLayout(m_layouts[id]->name());
        CentralLayout *central = centralActive ? centralActive : m_layouts[id];

        if (central->showInMenu() != menu) {
            central->setShowInMenu(menu);
        }

        if (central->disableBordersForMaximizedWindows() != disabledBorders) {
            central->setDisableBordersForMaximizedWindows(disabledBorders);
        }

        if (central->activities() != cleanedActivities) {
            central->setActivities(cleanedActivities);
        }

        //! If the layout name changed OR the layout path is a temporary one
        if (generic->name() != name || (id.startsWith("/tmp/"))) {
            //! If the layout is Active in MultipleLayouts
            if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts && generic->isActive()) {
                qDebug() << " Active Layout Should Be Renamed From : " << generic->name() << " TO :: " << name;
                activeLayoutsToRename[name] = generic;
            }

            QString tempFile = layoutTempDir.path() + "/" + QString(generic->name() + ".layout.latte");
            qDebug() << "new temp file ::: " << tempFile;

            if ((m_corona->layoutManager()->memoryUsage() == Types::SingleLayout) && (generic->name() == m_corona->layoutManager()->currentLayoutName())) {
                switchToLayout = name;
            }

            generic = m_layouts.take(id);
            delete generic;

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

        CentralLayout *nLayout = new CentralLayout(this, newFile);
        m_layouts[newFile] = nLayout;

        //! updating the #SETTINGSID in the model for the layout that was renamed
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

    if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        for (const auto &newLayoutName : activeLayoutsToRename.keys()) {
            Layout::GenericLayout *layout = activeLayoutsToRename[newLayoutName];
            qDebug() << " Active Layout of Type: " << layout->type() << " Is Renamed From : " << activeLayoutsToRename[newLayoutName]->name() << " TO :: " << newLayoutName;
            layout->renameLayout(newLayoutName);

            if (layout->type() == Layout::Type::Central) {
                CentralLayout *central = qobject_cast<CentralLayout *>(layout);

                if (central->activities().isEmpty()) {
                    //! that means it is an active layout for orphaned Activities
                    orphanedLayout = newLayoutName;
                }
            }

            //! broadcast the name change
            int row = rowForName(newLayoutName);
            QStandardItem *item = m_model->item(row, NAMECOLUMN);
            if (item) {
                emit itemChanged(item);
            }
        }
    }

    //! lock layouts in the end when the user has chosen it
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();
        bool locked = m_model->data(m_model->index(i, NAMECOLUMN), Qt::UserRole).toBool();

        Layout::GenericLayout *generic = m_corona->layoutManager()->layout(m_layouts[id]->name());
        Layout::GenericLayout *layout = generic ? generic : m_layouts[id];

        if (layout && locked && layout->isWritable()) {
            layout->lock();
        }
    }

    //! update SharedLayouts that are Active
    if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
        updateActiveShares();
    }

    //! reload layouts in layoutmanager
    m_corona->layoutManager()->loadLayouts();

    //! send to layout manager in which layout to switch
    Latte::Types::LayoutsMemoryUsage inMemoryOption = static_cast<Latte::Types::LayoutsMemoryUsage>(m_inMemoryButtons->checkedId());

    if (m_corona->layoutManager()->memoryUsage() != inMemoryOption) {
        Types::LayoutsMemoryUsage previousMemoryUsage = m_corona->layoutManager()->memoryUsage();
        m_corona->layoutManager()->setMemoryUsage(inMemoryOption);

        QVariant value = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), NAMECOLUMN), Qt::DisplayRole);
        QString layoutName = value.toString();

        m_corona->layoutManager()->switchToLayout(layoutName, previousMemoryUsage);
    } else {
        if (!switchToLayout.isEmpty()) {
            m_corona->layoutManager()->switchToLayout(switchToLayout);
        } else if (m_corona->layoutManager()->memoryUsage() == Types::MultipleLayouts) {
            m_corona->layoutManager()->syncMultipleLayoutsToActivities(orphanedLayout);
        }
    }

    return true;
}

void SettingsDialog::updateActiveShares()
{
    QHash<const QString, QStringList> currentSharesMap;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (isShared(i)) {
            QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
            QStringList shares = m_model->data(m_model->index(i, SHAREDCOLUMN), Qt::UserRole).toStringList();
            currentSharesMap[id] = shares;
        }
    }

    qDebug() << " CURRENT SHARES MAP :: " << currentSharesMap;

    QHash<CentralLayout *, SharedLayout *> unassign;

    //! CENTRAL (active) layouts that will become SHARED must be unloaded first
    for (QHash<const QString, QStringList>::iterator i=currentSharesMap.begin(); i!=currentSharesMap.end(); ++i) {
        CentralLayout *central = m_corona->layoutManager()->centralLayout(nameForId(i.key()));
        if (central) {
            m_corona->layoutManager()->unloadCentralLayout(central);
        }
    }

    //! CENTRAL (active) layouts that update their (active) SHARED layouts
    //! AND load SHARED layouts that are NOT ACTIVE
    for (QHash<const QString, QStringList>::iterator i=currentSharesMap.begin(); i!=currentSharesMap.end(); ++i) {
        SharedLayout *shared = m_corona->layoutManager()->sharedLayout(nameForId(i.key()));
        qDebug() << " SHARED :: " << nameForId(i.key());
        for (const auto &centralId : i.value()) {
            CentralLayout *central = m_corona->layoutManager()->centralLayout(nameForId(centralId));
            qDebug() << " CENTRAL NAME :: " << nameForId(centralId);
            if (central) {
                //! Assign this Central Layout at a different Shared Layout
                SharedLayout *oldShared = central->sharedLayout();

                if (!shared) {
                    //Shared not loaded and it must be loaded before proceed
                    m_corona->layoutManager()->registerAtSharedLayout(central, nameForId(i.key()));
                    shared = m_corona->layoutManager()->sharedLayout(nameForId(i.key()));
                }

                if (shared != oldShared) {
                    shared->addCentralLayout(central);
                    central->setSharedLayout(shared);
                    if (oldShared) {
                        //! CENTRAL layout that changed from one ACTIVESHARED layout to another
                        unassign[central] = shared;
                    }
                }
            }
        }
    }

    //! CENTRAL Layouts that wont have any SHARED Layout any more
    for (QHash<const QString, QStringList>::iterator i=m_sharesMap.begin(); i!=m_sharesMap.end(); ++i) {
        for (const auto &centralId : i.value()) {
            if (!mapHasRecord(centralId, currentSharesMap)) {
                CentralLayout *central = m_corona->layoutManager()->centralLayout(nameForId(centralId));
                if (central && central->sharedLayout()) {
                    central->sharedLayout()->removeCentralLayout(central);
                    central->setSharedLayoutName(QString());
                    central->setSharedLayout(nullptr);
                }
            }
        }
    }

    //! Unassing from Shared Layouts Central ones that are not assigned any more
    //! IMPORTANT: This must be done after all the ASSIGNMENTS in order to avoid
    //! to unload a SharedLayout that it should not
    for (QHash<CentralLayout *, SharedLayout *>::iterator i=unassign.begin(); i!=unassign.end(); ++i) {
        i.value()->removeCentralLayout(i.key());
    }

    //! TODO : (active) SharedLayouts that become Active should be unloaded first
    m_sharesMap.clear();
    m_sharesMap = currentSharesMap;
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

bool SettingsDialog::mapHasRecord(const QString &record, QHash<const QString, QStringList> &map)
{
    for (QHash<const QString, QStringList>::iterator i=map.begin(); i!=map.end(); ++i) {
        if (i.value().contains(record)) {
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

bool SettingsDialog::isMenuCell(int column) const
{
    return column == MENUCOLUMN;
}

bool SettingsDialog::isShared(int row) const
{
    if (row >=0 ) {
        QStringList shares = m_model->data(m_model->index(row, SHAREDCOLUMN), Qt::UserRole).toStringList();
        if (!shares.isEmpty()) {
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

int SettingsDialog::rowForId(QString id) const
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowId = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();

        if (rowId == id) {
            return i;
        }
    }

    return -1;
}

int SettingsDialog::rowForName(QString layoutName) const
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowName = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        if (rowName == layoutName) {
            return i;
        }
    }

    return -1;
}

QString SettingsDialog::idForRow(int row) const
{
    return m_model->data(m_model->index(row, IDCOLUMN), Qt::DisplayRole).toString();
}

QString SettingsDialog::nameForId(QString id) const
{
    int row = rowForId(id);
    return m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole).toString();
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

