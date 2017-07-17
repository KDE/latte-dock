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
    m_model->setHorizontalHeaderItem(3, new QStandardItem(QString(i18n("Menu"))));
    m_model->setHorizontalHeaderItem(4, new QStandardItem(QString(i18n("Activities"))));

    ui->layoutsView->setModel(m_model);

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

    setVisible(false);
}

void LayoutConfigDialog::reject()
{
    qDebug() << Q_FUNC_INFO;

    setVisible(false);
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

        QStandardItem *color = new QStandardItem(layoutSets.color());
        m_model->setItem(i - 1, 1, color);

        QStandardItem *name = new QStandardItem(layoutSets.name());
        m_model->setItem(i - 1, 2, name);

        QString menuText = layoutSets.showInMenu() ? "true" : "false";
        QStandardItem *menu = new QStandardItem(menuText);
        m_model->setItem(i - 1, 3, menu);

        QStandardItem *activities = new QStandardItem(layoutSets.activities().join(","));
        m_model->setItem(i - 1, 4, activities);
    }
}

}//end of namespace

