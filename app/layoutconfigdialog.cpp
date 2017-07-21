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
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTemporaryDir>

#include <KLocalizedString>
#include <KNotification>

namespace Latte {

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

    m_model->setHorizontalHeaderItem(0, new QStandardItem(QString("#path")));
    m_model->setHorizontalHeaderItem(1, new QStandardItem(QString(i18n("Color"))));
    m_model->setHorizontalHeaderItem(2, new QStandardItem(QString(i18n("Name"))));
    m_model->setHorizontalHeaderItem(3, new QStandardItem(QString(i18n("In Menu"))));
    m_model->setHorizontalHeaderItem(4, new QStandardItem(QString(i18n("Activities"))));

    ui->layoutsView->setModel(m_model);
    ui->layoutsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->layoutsView->horizontalHeader()->setStretchLastSection(true);
    ui->layoutsView->verticalHeader()->setVisible(false);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    //! this line should be commented for debugging layouts window functionality
    //ui->layoutsView->setColumnHidden(0, true);

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

    ui->layoutsView->setItemDelegateForColumn(1, new ColorCmbBoxDelegate(this, iconsPath, colors));
    ui->layoutsView->setItemDelegateForColumn(3, new CheckBoxDelegate(this));
    ui->layoutsView->setItemDelegateForColumn(4, new ActivityCmbBoxDelegate(this));

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

void LayoutConfigDialog::on_copyButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString tempDir = uniqueTempDirectory();

    QString id = m_model->data(m_model->index(row, 0), Qt::DisplayRole).toString();
    QString color = m_model->data(m_model->index(row, 1), Qt::BackgroundRole).toString();
    QString layoutName = m_model->data(m_model->index(row, 2), Qt::DisplayRole).toString();
    bool menu = m_model->data(m_model->index(row, 3), Qt::CheckStateRole).toInt() == Qt::Checked ? true : false;

    QString copiedId = tempDir + "/" + layoutName + ".layout.latte";
    QFile(id).copy(copiedId);

    LayoutSettings *settings = new LayoutSettings(this, copiedId);
    m_layouts[copiedId] = settings;

    insertLayoutInfoAtRow(row + 1, copiedId, color, layoutName, menu, QStringList());
}

void LayoutConfigDialog::on_removeButton_clicked()
{
    qDebug() << Q_FUNC_INFO;

    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString layoutName = m_model->data(m_model->index(row, 2), Qt::DisplayRole).toString();

    if (layoutName == m_manager->currentLayoutName()) {
        return;
    }

    m_model->removeRow(row);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void LayoutConfigDialog::on_importButton_clicked()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::on_exportButton_clicked()
{
    int row = ui->layoutsView->currentIndex().row();

    if (row < 0) {
        return;
    }

    QString layoutExported = m_model->data(m_model->index(row, 0), Qt::DisplayRole).toString();

    qDebug() << Q_FUNC_INFO;

    QFileDialog *m_fileDialog = new QFileDialog(this, i18nc("export layout", "Export layout")
            , QDir::homePath() , QStringLiteral("layout.latte"));

    m_fileDialog->setFileMode(QFileDialog::AnyFile);
    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    m_fileDialog->setDefaultSuffix("layout.latte");
    m_fileDialog->setNameFilter(i18nc("export layout", "Latte Dock layout file")
                                + "(*.layout.latte)");

    connect(m_fileDialog, &QFileDialog::finished
            , m_fileDialog, &QFileDialog::deleteLater);

    connect(m_fileDialog, &QFileDialog::fileSelected
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
    });


    m_fileDialog->open();
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
}

void LayoutConfigDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::loadLayouts()
{
    m_initLayoutPaths.clear();
    m_model->clear();

    int i = 0;

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
    }

    recalculateAvailableActivities();
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
    menuItem->setCheckable(true);
    menuItem->setCheckState(menu ? Qt::Checked : Qt::Unchecked);

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

    m_model->setData(m_model->index(row, 0), path, Qt::DisplayRole);

    m_model->setData(m_model->index(row, 1), color, Qt::BackgroundRole);

    QFont font;

    if (name == m_manager->currentLayoutName()) {
        font.setBold(true);
    } else {
        font.setBold(false);
    }

    m_model->setData(m_model->index(row, 2), QVariant(name), Qt::DisplayRole);
    m_model->setData(m_model->index(row, 2), font, Qt::FontRole);

    m_model->setData(m_model->index(row, 4), activities, Qt::UserRole);
}


void LayoutConfigDialog::on_switchButton_clicked()
{
    QVariant value = m_model->data(m_model->index(ui->layoutsView->currentIndex().row(), 2), Qt::DisplayRole);

    if (value.isValid()) {
        m_manager->switchToLayout(value.toString());
    } else {
        qDebug() << "not valid layout";
    }
}

void LayoutConfigDialog::currentLayoutNameChanged()
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex nameIndex = m_model->index(i, 2);
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

    if (item->column() == 4) {
        //! recalculate the available activities
        recalculateAvailableActivities();
    }
}

void LayoutConfigDialog::currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QString id = m_model->data(m_model->index(current.row(), 0), Qt::DisplayRole).toString();

    if (m_layouts[id]->name() == m_manager->currentLayoutName()) {
        ui->removeButton->setEnabled(false);
    } else {
        ui->removeButton->setEnabled(true);
    }
}

void LayoutConfigDialog::recalculateAvailableActivities()
{
    QStringList tempActivities = m_manager->activities();

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStringList assigned = m_model->data(m_model->index(i, 4), Qt::UserRole).toStringList();

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
        QString layout1 = m_model->data(m_model->index(i, 2), Qt::DisplayRole).toString();

        for (int j = i + 1; j < m_model->rowCount(); ++j) {
            QString temp = m_model->data(m_model->index(j, 2), Qt::DisplayRole).toString();

            //!same layout name exists again
            if (layout1 == temp) {
                auto msg = new QMessageBox(this);
                msg->setIcon(QMessageBox::Warning);
                msg->setWindowTitle(i18n("Layout Warning"));
                msg->setText(i18n("There are layouts with the same name, that is not permitted!!! Please update these names to re-apply the changes..."));
                msg->setStandardButtons(QMessageBox::Ok);

                connect(msg, &QMessageBox::finished, this, [ &, i, j](int result) {
                    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect;
                    QModelIndex indexBase = m_model->index(i, 2);
                    ui->layoutsView->selectionModel()->select(indexBase, flags);

                    QModelIndex indexOccurence = m_model->index(j, 2);
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
        QString id = m_model->data(m_model->index(i, 0), Qt::DisplayRole).toString();
        QString color = m_model->data(m_model->index(i, 1), Qt::BackgroundRole).toString();
        QString name = m_model->data(m_model->index(i, 2), Qt::DisplayRole).toString();
        bool menu = m_model->data(m_model->index(i, 3), Qt::CheckStateRole).toInt() == Qt::Checked ? true : false;
        QStringList lActivities = m_model->data(m_model->index(i, 4), Qt::UserRole).toStringList();

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

        if (layout->name() != name) {
            QString tempFile = layoutTempDir.filePath(QString(layout->name() + ".layout.latte"));
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
            QString tId = m_model->data(m_model->index(j, 0), Qt::DisplayRole).toString();

            if (tId == fromRenamePaths[i]) {
                m_model->setData(m_model->index(j, 0), newFile, Qt::DisplayRole);
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
        QString rowId = m_model->data(m_model->index(i, 0), Qt::DisplayRole).toString();

        if (rowId == id) {
            return true;
        }
    }

    return false;
}

QString LayoutConfigDialog::uniqueTempDirectory()
{
    QTemporaryDir tempDir;
    tempDir.setAutoRemove(false);
    m_tempDirectories.append(tempDir.path());

    return tempDir.path();
}

}//end of namespace

