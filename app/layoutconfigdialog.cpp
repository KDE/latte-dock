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

#include <QDir>
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

    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked
            , this, &LayoutConfigDialog::apply);
    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &LayoutConfigDialog::restoreDefaults);

    m_model = new QStandardItemModel(manager->layouts().count(), 5, this);

    m_model->setHorizontalHeaderItem(0, new QStandardItem(QString("id")));
    m_model->setHorizontalHeaderItem(1, new QStandardItem(QString(i18n("Color"))));
    m_model->setHorizontalHeaderItem(2, new QStandardItem(QString(i18n("Name"))));
    m_model->setHorizontalHeaderItem(3, new QStandardItem(QString(i18n("In Menu"))));
    m_model->setHorizontalHeaderItem(4, new QStandardItem(QString(i18n("Activities"))));

    ui->layoutsView->setModel(m_model);

    connect(m_manager, &LayoutManager::currentLayoutNameChanged, this, &LayoutConfigDialog::currentLayoutNameChanged);

    loadLayouts();
}

LayoutConfigDialog::~LayoutConfigDialog()
{
    qDebug() << Q_FUNC_INFO;

    if (m_model) {
        delete m_model;
    }
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
        m_model->setItem(i - 1, 0, id);

        QStandardItem *color = new QStandardItem();
        color->setEditable(false);
        m_model->setItem(i - 1, 1, color);
        m_model->setData(m_model->index(i - 1, 1), QColor(layoutSets.color()), Qt::BackgroundRole);

        QStandardItem *name = new QStandardItem(layoutSets.name());

        QFont font;

        if (layoutSets.name() == m_manager->currentLayoutName()) {
            font.setBold(true);
        } else {
            font.setBold(false);
        }

        m_model->setItem(i - 1, 2, name);
        m_model->setData(m_model->index(i - 1, 2), font, Qt::FontRole);

        //QString menuText = layoutSets.showInMenu() ? QString::fromUtf8("\u2714") : "";
        QStandardItem *menu = new QStandardItem();
        menu->setTextAlignment(Qt::AlignCenter);
        menu->setEditable(false);
        menu->setCheckable(true);
        menu->setCheckState(layoutSets.showInMenu() ? Qt::Checked : Qt::Unchecked);
        m_model->setItem(i - 1, 3, menu);

        QStandardItem *activities = new QStandardItem(layoutSets.activities().join(","));
        activities->setEditable(false);
        m_model->setItem(i - 1, 4, activities);
    }
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
            } else {
                font.setBold(false);
            }

            m_model->setData(nameIndex, font, Qt::FontRole);
        }
    }
}

}//end of namespace

