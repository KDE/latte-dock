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

#include <QDir>
#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>

#include <KLocalizedString>

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

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked
            , this, &LayoutConfigDialog::apply);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &LayoutConfigDialog::restoreDefaults);

    m_model = new QStandardItemModel(manager->layouts().count(), 5, this);

    m_model->setHorizontalHeaderItem(0, new QStandardItem(QString("#")));
    m_model->setHorizontalHeaderItem(1, new QStandardItem(QString(i18n("Color"))));
    m_model->setHorizontalHeaderItem(2, new QStandardItem(QString(i18n("Name"))));
    m_model->setHorizontalHeaderItem(3, new QStandardItem(QString(i18n("In Menu"))));
    m_model->setHorizontalHeaderItem(4, new QStandardItem(QString(i18n("Activities"))));

    ui->layoutsView->setModel(m_model);
    ui->layoutsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->layoutsView->horizontalHeader()->setStretchLastSection(true);
    ui->layoutsView->verticalHeader()->setVisible(false);

    //! this line should be commented for debugging layouts window functionality
    ui->layoutsView->setColumnHidden(0, true);

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
}

LayoutConfigDialog::~LayoutConfigDialog()
{
    qDebug() << Q_FUNC_INFO;

    if (m_model) {
        delete m_model;
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
}

void LayoutConfigDialog::on_removeButton_clicked()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::on_importButton_clicked()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::on_exportButton_clicked()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::accept()
{
    qDebug() << Q_FUNC_INFO;

    //setVisible(false);
    deleteLater();
}

void LayoutConfigDialog::reject()
{
    qDebug() << Q_FUNC_INFO;

    //setVisible(false);
    deleteLater();
}

void LayoutConfigDialog::apply()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;
}

void LayoutConfigDialog::loadLayouts()
{
    int i = 0;

    foreach (auto layout, m_manager->layouts()) {
        i++;
        LayoutSettings layoutSets(this, QDir::homePath() + "/.config/latte/" + layout + ".layout.latte");

        qDebug() << i << ". " << layoutSets.name() << " - " << layoutSets.color() << " - "
                 << layoutSets.showInMenu() << " - " << layoutSets.activities();

        QStandardItem *id = new QStandardItem(QString::number(i));
        id->setTextAlignment(Qt::AlignCenter);
        m_model->setItem(i - 1, 0, id);

        QStandardItem *color = new QStandardItem();

        color->setSelectable(false);
        m_model->setItem(i - 1, 1, color);
        m_model->setData(m_model->index(i - 1, 1), layoutSets.color(), Qt::BackgroundRole);

        QStandardItem *name = new QStandardItem(layoutSets.name());

        QFont font;

        if (layoutSets.name() == m_manager->currentLayoutName()) {
            font.setBold(true);
        } else {
            font.setBold(false);
        }

        name->setTextAlignment(Qt::AlignCenter);

        m_model->setItem(i - 1, 2, name);
        m_model->setData(m_model->index(i - 1, 2), font, Qt::FontRole);

        QStandardItem *menu = new QStandardItem();
        menu->setEditable(false);
        menu->setSelectable(true);
        menu->setCheckable(true);
        menu->setCheckState(layoutSets.showInMenu() ? Qt::Checked : Qt::Unchecked);
        m_model->setItem(i - 1, 3, menu);

        QStandardItem *activities = new QStandardItem(layoutSets.activities().join(","));
        m_model->setItem(i - 1, 4, activities);
        m_model->setData(m_model->index(i - 1, 4), layoutSets.activities(), Qt::UserRole);

        if (layoutSets.name() == m_manager->currentLayoutName()) {
            ui->layoutsView->selectRow(i - 1);
        }
    }

    recalculateAvailableActivities();
}

void LayoutConfigDialog::on_switchButton_clicked()
{
    QString sRec = "not valid column";
    QVariant value = m_model->data(ui->layoutsView->currentIndex(), 2);

    if (value.isValid()) {
        m_manager->switchToLayout(value.toString());
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
    //! recalculate the available activities
    if (item->column() == 4) {
        recalculateAvailableActivities();
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

}//end of namespace

