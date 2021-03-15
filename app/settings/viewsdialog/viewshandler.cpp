/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "viewshandler.h"

// local
#include "ui_viewsdialog.h"
#include "viewsdialog.h"
#include "../settingsdialog/layoutscontroller.h"
#include "../settingsdialog/layoutsmodel.h"
#include "../settingsdialog/delegates/layoutcmbitemdelegate.h"
#include "../../data/layoutstable.h"
#include "../../layout/abstractlayout.h"

// Qt
#include <QMessageBox>

namespace Latte {
namespace Settings {
namespace Handler {

ViewsHandler::ViewsHandler(Dialog::ViewsDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui())
{
    init();
}

ViewsHandler::~ViewsHandler()
{
}

void ViewsHandler::init()
{
    //! Layouts
    m_layoutsProxyModel = new QSortFilterProxyModel(this);
    m_layoutsProxyModel->setSourceModel(m_dialog->layoutsController()->baseModel());
    m_layoutsProxyModel->setSortRole(Model::Layouts::SORTINGROLE);
    m_layoutsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_layoutsProxyModel->sort(Model::Layouts::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->layoutsCmb->setModel(m_layoutsProxyModel);
    m_ui->layoutsCmb->setModelColumn(Model::Layouts::NAMECOLUMN);
    m_ui->layoutsCmb->setItemDelegate(new Settings::Layout::Delegate::LayoutCmbItemDelegate(this));

    connect(this, &ViewsHandler::currentLayoutChanged, this, &ViewsHandler::reload);

    reload();

    //! connect layout combobox after the selected layout has been loaded
    connect(m_ui->layoutsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewsHandler::onCurrentLayoutIndexChanged);

    //! data were changed
    connect(this, &ViewsHandler::dataChanged, this, [&]() {
        loadLayout(c_data);
    });
}

void ViewsHandler::reload()
{
    o_data = m_dialog->layoutsController()->selectedLayoutCurrentData();
    c_data = o_data;

    m_ui->layoutsCmb->setCurrentText(o_data.name);

    loadLayout(c_data);
}

void ViewsHandler::loadLayout(const Latte::Data::Layout &data)
{
    updateWindowTitle();
}

Latte::Data::Layout ViewsHandler::currentData() const
{
    return c_data;
}

bool ViewsHandler::hasChangedData() const
{
    return o_data != c_data;
}

bool ViewsHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void ViewsHandler::reset()
{
    c_data = o_data;
    emit currentLayoutChanged();
}

void ViewsHandler::resetDefaults()
{
    //do nothing
}

void ViewsHandler::save()
{
    m_dialog->layoutsController()->setLayoutProperties(currentData());
}

void ViewsHandler::onCurrentLayoutIndexChanged(int row)
{
    bool switchtonewlayout{true};

    if (hasChangedData()) {
        int result = saveChanges();

        if (result == QMessageBox::Apply) {
            save();
        } else if (result == QMessageBox::Discard) {
            //do nothing
        } else if (result == QMessageBox::Cancel) {
            switchtonewlayout = false;
        }
    }

    if (switchtonewlayout) {
        QString layoutId = m_layoutsProxyModel->data(m_layoutsProxyModel->index(row, Model::Layouts::IDCOLUMN), Qt::UserRole).toString();
        m_dialog->layoutsController()->selectRow(layoutId);
        reload();

        emit currentLayoutChanged();
    } else {
        //! reset combobox index
        m_ui->layoutsCmb->setCurrentText(c_data.name);
    }
}

void ViewsHandler::updateWindowTitle()
{
    m_dialog->setWindowTitle(m_ui->layoutsCmb->currentText());
}

int ViewsHandler::saveChanges()
{
    if (hasChangedData()) {
        QString layoutName = c_data.name;
        QString saveChangesText = i18n("The settings of <b>%0</b> layout have changed. Do you want to apply the changes or discard them?").arg(layoutName);

        return m_dialog->saveChangesConfirmation(saveChangesText);
    }

    return QMessageBox::Cancel;
}

}
}
}
