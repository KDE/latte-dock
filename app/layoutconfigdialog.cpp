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

#include "ui_layoutconfigdialog.h"
#include "layoutconfigdialog.h"
#include "layoutsettings.h"
#include "layoutsDelegates/checkboxdelegate.h"
#include "layoutsDelegates/colorcmbboxdelegate.h"
#include "layoutsDelegates/activitycmbboxdelegate.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QProcess>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTemporaryDir>

#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KLocalizedString>
#include <KNotification>


namespace Latte {

const int IDCOLUMN = 0;
const int COLORCOLUMN = 1;
const int NAMECOLUMN = 2;
const int MENUCOLUMN = 3;
const int ACTIVITYCOLUMN = 4;
const QChar CheckMark{0x2714};

LayoutConfigDialog::LayoutConfigDialog(QWidget *parent, LayoutManager *manager)
    : QDialog(parent),
      ui(new Ui::LayoutConfigDialog),
      m_manager(manager)
{
    ui->setupUi(this);

    setWindowTitle(i18n("Layouts Editor"));

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    resize(m_manager->corona()->universalSettings()->layoutsWindowSize());

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked
            , this, &LayoutConfigDialog::apply);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &LayoutConfigDialog::restoreDefaults);

    m_model = new QStandardItemModel(manager->layouts().count(), 5, this);

    ui->layoutsView->setModel(m_model);
    ui->layoutsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->layoutsView->horizontalHeader()->setStretchLastSection(true);
    ui->layoutsView->verticalHeader()->setVisible(false);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    connect(m_manager, &LayoutManager::currentLayoutNameChanged, this, &LayoutConfigDialog::currentLayoutNameChanged);

    loadLayouts();

    QString iconsPath(m_manager->corona()->kPackage().path() + "../../plasmoids/org.kde.latte.containment/contents/icons/");

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

    ui->layoutsView->setItemDelegateForColumn(COLORCOLUMN, new ColorCmbBoxDelegate(this, iconsPath, colors));
    ui->layoutsView->setItemDelegateForColumn(MENUCOLUMN, new CheckBoxDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(ACTIVITYCOLUMN, new ActivityCmbBoxDelegate(this));

    ui->newButton->setText(i18nc("new button", "New"));
    ui->copyButton->setText(i18nc("copy button", "Copy"));
    ui->removeButton->setText(i18nc("remove button", "Remove"));
    ui->switchButton->setText(i18nc("switch button", "Switch"));
    ui->importButton->setText(i18nc("import button", "Import"));
    ui->exportButton->setText(i18nc("export button", "Export"));

    connect(m_model, &QStandardItemModel::itemChanged, this, &LayoutConfigDialog::itemChanged);
    connect(ui->layoutsView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &LayoutConfigDialog::currentRowChanged);
}

LayoutConfigDialog::~LayoutConfigDialog()
{
    qDebug() << Q_FUNC_INFO;

    qDeleteAll(m_layouts);

    if (m_model) {
        delete m_model;
    }

    if (m_manager && m_manager->corona() && m_manager->corona()->universalSettings()) {
        m_manager->corona()->universalSettings()->setLayoutsWindowSize(size());
    }

    foreach (auto tempDir, m_tempDirectories) {
        QDir tDir(tempDir);

        if (tDir.exists() && tempDir.startsWith("/tmp/")) {
            tDir.removeRecursively();
        }
    }
}

QStringList LayoutConfigDialog::activities()
{
    return m_manager->activities();
}

QStringList LayoutConfigDialog::availableActivities()
{
    return m_availableActivities;
}

void LayoutConfigDialog::on_newButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    //! find Default preset path
    foreach (auto preset, m_manager->presetsPaths()) {
        QString presetName = LayoutSettings::layoutName(preset);

        if (presetName == "Default") {
            QByteArray presetNameChars = presetName.toUtf8();
            const char *prset_str = presetNameChars.data();
            presetName = uniqueLayoutName(i18n(prset_str));

            addLayoutForFile(preset, presetName, true, false);
            break;
        }
    }
}

void LayoutConfigDialog::on_copyButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString tempDir = uniqueTempDirectory();

    QString id = m_model->data(m_model->index(row, IDCOLUMN), Qt::DisplayRole).toString();
    QString color = m_model->data(m_model->index(row, COLORCOLUMN), Qt::BackgroundRole).toString();
    QString layoutName = uniqueLayoutName(m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole).toString());
    bool menu = m_model->data(m_model->index(row, MENUCOLUMN), Qt::DisplayRole).toString() == CheckMark;

    QString copiedId = tempDir + "/" + layoutName + ".layout.latte";
    QFile(id).copy(copiedId);

    LayoutSettings *settings = new LayoutSettings(this, copiedId);
    m_layouts[copiedId] = settings;

    insertLayoutInfoAtRow(row + 1, copiedId, color, layoutName, menu, QStringList());

    ui->layoutsView->selectRow(row + 1);
}

void LayoutConfigDialog::on_removeButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString layoutName = m_model->data(m_model->index(row, NAMECOLUMN), Qt::DisplayRole).toString();

    if (layoutName == m_manager->currentLayoutName()) {
        return;
    }

    m_model->removeRow(row);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);


    row = qMax(row - 1, 0);

    ui->layoutsView->selectRow(row);
}

void LayoutConfigDialog::on_importButton_clicked()
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
                i18n("You are going to import an old version <b>v0.1</b> configuration file.<br><b>Be careful</b>, importing the entire configuration <b>will erase all</b> your current configuration!!!.<br><br> <i>Alternative, you can <b>import safely</b> from this file<br><b>only the contained layouts...</b></i>"));
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
                QProcess::startDetached(qGuiApp->applicationFilePath() + " --import \"" + file + "\"");
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
                    QProcess::startDetached(qGuiApp->applicationFilePath() + " --import \"" + file + "\"");
                    qGuiApp->exit();
                }
            });

            msg->open();
        }
    });

    fileDialog->open();
}

bool LayoutConfigDialog::importLayoutsFromV1ConfigFile(QString file)
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
            if (m_manager->importer()->importOldLayout(applets, name, false, tempDir.absolutePath())) {
                addLayoutForFile(tempDir.absolutePath() + "/" + name + ".layout.latte", name, false);
            }

            QString alternativeName = name + "-" + i18nc("layout", "Alternative");

            if (m_manager->importer()->importOldLayout(applets, alternativeName, false, tempDir.absolutePath())) {
                addLayoutForFile(tempDir.absolutePath() + "/" + alternativeName + ".layout.latte", alternativeName, false);
            }
        }

        return true;
    }

    return false;
}


void LayoutConfigDialog::on_exportButton_clicked()
{
    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString layoutExported = m_model->data(m_model->index(row, IDCOLUMN), Qt::DisplayRole).toString();

    qDebug() << Q_FUNC_INFO;

    QFileDialog *fileDialog = new QFileDialog(this, i18nc("export layout/configuration", "Export Layout/Configuration")
            , QDir::homePath() , QStringLiteral("layout.latte"));

    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    fileDialog->setDefaultSuffix("layout.latte");

    QStringList filters;
    QString filter1(i18nc("export layout", "Latte Dock Layout file v0.2") + "(*.layout.latte)");
    QString filter2(i18nc("export full configuraion", "Latte Dock Full Configuration file v0.2") + "(*.latterc)");

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

            LayoutSettings layoutS(this, file);
            layoutS.setActivities(QStringList());

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

            if (m_manager->importer()->exportFullConfiguration(file)) {

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

void LayoutConfigDialog::accept()
{
    qDebug() << Q_FUNC_INFO;

    //setVisible(false);
    if (saveAllChanges()) {
        deleteLater();
    }
}

void LayoutConfigDialog::reject()
{
    qDebug() << Q_FUNC_INFO;

    deleteLater();
}

void LayoutConfigDialog::apply()
{
    qDebug() << Q_FUNC_INFO;
    saveAllChanges();
    updateButtonsState();
}

void LayoutConfigDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;

    foreach (auto preset, m_manager->presetsPaths()) {
        QString presetName = LayoutSettings::layoutName(preset);
        QByteArray presetNameChars = presetName.toUtf8();
        const char *prset_str = presetNameChars.data();
        presetName = i18n(prset_str);

        if (!nameExistsInModel(presetName)) {
            addLayoutForFile(preset, presetName);
        }
    }
}

void LayoutConfigDialog::addLayoutForFile(QString file, QString layoutName, bool newTempDirectory, bool showNotification)
{
    if (layoutName.isEmpty()) {
        layoutName = LayoutSettings::layoutName(file);
    }

    QString copiedId;

    if (newTempDirectory) {
        QString tempDir = uniqueTempDirectory();
        copiedId = tempDir + "/" + layoutName + ".layout.latte";
        QFile(file).copy(copiedId);
    } else {
        copiedId = file;
    }

    LayoutSettings *settings = new LayoutSettings(this, copiedId);
    m_layouts[copiedId] = settings;

    QString id = copiedId;
    QString color = settings->color();
    bool menu = settings->showInMenu();

    layoutName = uniqueLayoutName(layoutName);

    int row = ascendingRowFor(layoutName);
    insertLayoutInfoAtRow(row, copiedId, color, layoutName, menu, QStringList());

    ui->layoutsView->selectRow(row);

    if (showNotification) {
        //NOTE: The pointer is automatically deleted when the event is closed
        auto notification = new KNotification("import-done", KNotification::CloseOnTimeout);
        notification->setText(i18nc("import-done", "Layout: <b>%0</b> imported successfully\n").arg(layoutName));
        notification->sendEvent();
    }
}

void LayoutConfigDialog::loadLayouts()
{
    m_initLayoutPaths.clear();
    m_model->clear();

    int i = 0;
    QStringList brokenLayouts;

    foreach (auto layout, m_manager->layouts()) {
        QString layoutPath = QDir::homePath() + "/.config/latte/" + layout + ".layout.latte";
        m_initLayoutPaths.append(layoutPath);

        LayoutSettings *layoutSets = new LayoutSettings(this, layoutPath);
        m_layouts[layoutPath] = layoutSets;

        insertLayoutInfoAtRow(i, layoutPath, layoutSets->color(), layoutSets->name(),
                              layoutSets->showInMenu(), layoutSets->activities());

        qDebug() << "counter:" << i << " total:" << m_model->rowCount();

        i++;

        if (layoutSets->name() == m_manager->currentLayoutName()) {
            ui->layoutsView->selectRow(i - 1);
        }

        if (layoutSets->fileIsBroken()) {
            brokenLayouts.append(layoutSets->name());
        }
    }

    recalculateAvailableActivities();

    m_model->setHorizontalHeaderItem(IDCOLUMN, new QStandardItem(QString("#path")));
    m_model->setHorizontalHeaderItem(COLORCOLUMN, new QStandardItem(QString(i18n("Color"))));
    m_model->setHorizontalHeaderItem(NAMECOLUMN, new QStandardItem(QString(i18n("Name"))));
    m_model->setHorizontalHeaderItem(MENUCOLUMN, new QStandardItem(QString(i18n("In Menu"))));
    m_model->setHorizontalHeaderItem(ACTIVITYCOLUMN, new QStandardItem(QString(i18n("Activities"))));

    //! this line should be commented for debugging layouts window functionality
    ui->layoutsView->setColumnHidden(IDCOLUMN, true);

    //! there are broken layouts and the user must be informed!
    if (brokenLayouts.count() > 0) {
        auto msg = new QMessageBox(this);
        msg->setIcon(QMessageBox::Warning);
        msg->setWindowTitle(i18n("Layout Warning"));
        msg->setText(i18n("The layout(s) <b>%0</b> have <i>broken configuration</i>!!! Please <b>remove them<b> to improve the system stability...").arg(brokenLayouts.join(",")));
        msg->setStandardButtons(QMessageBox::Ok);

        msg->open();
    }
}

void LayoutConfigDialog::insertLayoutInfoAtRow(int row, QString path, QString color, QString name, bool menu, QStringList activities)
{
    QStandardItem *pathItem = new QStandardItem(path);

    QStandardItem *colorItem = new QStandardItem();
    colorItem->setSelectable(false);

    QStandardItem *nameItem = new QStandardItem(name);
    nameItem->setTextAlignment(Qt::AlignCenter);

    QStandardItem *menuItem = new QStandardItem();
    menuItem->setEditable(false);
    menuItem->setSelectable(true);
    menuItem->setText(menu ? CheckMark : QString());
    menuItem->setTextAlignment(Qt::AlignCenter);

    QStandardItem *activitiesItem = new QStandardItem(activities.join(","));

    QList<QStandardItem *> items;

    items.append(pathItem);
    items.append(colorItem);
    items.append(nameItem);
    items.append(menuItem);
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

    QFont font;

    if (name == m_manager->currentLayoutName()) {
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

    m_model->setData(m_model->index(row, ACTIVITYCOLUMN), activities, Qt::UserRole);
}


void LayoutConfigDialog::on_switchButton_clicked()
{
    QVariant value = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), NAMECOLUMN), Qt::DisplayRole);

    if (value.isValid()) {
        m_manager->switchToLayout(value.toString());
    } else {
        qDebug() << "not valid layout";
    }
}

void LayoutConfigDialog::currentLayoutNameChanged()
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex nameIndex = m_model->index(i, NAMECOLUMN);
        QVariant value = m_model->data(nameIndex);

        if (value.isValid()) {
            QFont font;

            if (m_manager->currentLayoutName() == value.toString()) {
                font.setBold(true);
                ui->layoutsView->selectRow(i);
            } else {
                font.setBold(false);
            }

            m_model->setData(nameIndex, font, Qt::FontRole);
        }
    }
}

void LayoutConfigDialog::itemChanged(QStandardItem *item)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);

    if (item->column() == ACTIVITYCOLUMN) {
        //! recalculate the available activities
        recalculateAvailableActivities();
    }
}

void LayoutConfigDialog::currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    updateButtonsState();
}

void LayoutConfigDialog::updateButtonsState()
{
    QString id = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), IDCOLUMN), Qt::DisplayRole).toString();

    if (m_layouts[id]->name() == m_manager->currentLayoutName()) {
        ui->removeButton->setEnabled(false);
    } else {
        ui->removeButton->setEnabled(true);
    }

    if (id.startsWith("/tmp/")) {
        ui->switchButton->setEnabled(false);
    } else {
        ui->switchButton->setEnabled(true);
    }
}

void LayoutConfigDialog::recalculateAvailableActivities()
{
    QStringList tempActivities = m_manager->activities();

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

bool LayoutConfigDialog::dataAreAccepted()
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

bool LayoutConfigDialog::saveAllChanges()
{
    if (!dataAreAccepted()) {
        return false;
    }

    QStringList knownActivities = activities();

    QTemporaryDir layoutTempDir;

    qDebug() << "Temporary Directory ::: " << layoutTempDir.path();

    QStringList fromRenamePaths;
    QStringList toRenamePaths;
    QStringList toRenameNames;

    QString switchToLayout;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString color = m_model->data(m_model->index(i, COLORCOLUMN), Qt::BackgroundRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();
        bool menu = m_model->data(m_model->index(i, MENUCOLUMN), Qt::DisplayRole).toString() == CheckMark;
        QStringList lActivities = m_model->data(m_model->index(i, ACTIVITYCOLUMN), Qt::UserRole).toStringList();

        QStringList cleanedActivities;

        //!update only activities that are valid
        foreach (auto activity, lActivities) {
            if (knownActivities.contains(activity)) {
                cleanedActivities.append(activity);
            }
        }

        //qDebug() << i << ". " << id << " - " << color << " - " << name << " - " << menu << " - " << lActivities;

        LayoutSettings *layout = name == m_manager->currentLayoutName() ? m_manager->currentLayout() : m_layouts[id];

        if (layout->color() != color) {
            layout->setColor(color);
        }

        if (layout->showInMenu() != menu) {
            layout->setShowInMenu(menu);
        }

        if (layout->activities() != cleanedActivities) {
            layout->setActivities(cleanedActivities);
        }

        //!if the layout name changed or when the layout path is a temporary one
        if (layout->name() != name || (id.startsWith("/tmp/"))) {
            QString tempFile = layoutTempDir.path() + "/" + QString(layout->name() + ".layout.latte");
            qDebug() << "new temp file ::: " << tempFile;

            if (layout->name() == m_manager->currentLayoutName()) {
                switchToLayout = name;
                m_manager->corona()->unload();
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

        LayoutSettings *nLayout = new LayoutSettings(this, newFile);
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

    //! remove layouts that have been removed from the user
    foreach (auto initLayout, m_initLayoutPaths) {
        if (!idExistsInModel(initLayout)) {
            QFile(initLayout).remove();
        }
    }

    m_manager->loadLayouts();

    if (!switchToLayout.isNull()) {
        m_manager->switchToLayout(switchToLayout);
    }

    return true;
}

bool LayoutConfigDialog::idExistsInModel(QString id)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowId = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();

        if (rowId == id) {
            return true;
        }
    }

    return false;
}

bool LayoutConfigDialog::nameExistsInModel(QString name)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowName = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        if (rowName == name) {
            return true;
        }
    }

    return false;
}

int LayoutConfigDialog::ascendingRowFor(QString name)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString rowName = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        if (rowName.toUpper() > name.toUpper()) {
            return i;
        }
    }

    return m_model->rowCount();
}

QString LayoutConfigDialog::uniqueTempDirectory()
{
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    m_tempDirectories.append(tempDir.path());

    return tempDir.path();
}

QString LayoutConfigDialog::uniqueLayoutName(QString name)
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

