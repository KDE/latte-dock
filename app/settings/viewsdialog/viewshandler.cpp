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
#include "viewscontroller.h"
#include "viewsdialog.h"
#include "../settingsdialog/layoutscontroller.h"
#include "../settingsdialog/layoutsmodel.h"
#include "../settingsdialog/delegates/layoutcmbitemdelegate.h"
#include "../../data/layoutstable.h"
#include "../../data/genericbasictable.h"
#include "../../data/viewstable.h"
#include "../../lattecorona.h"
#include "../../layout/abstractlayout.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/manager.h"
#include "../../layouts/storage.h"
#include "../../layouts/synchronizer.h"
#include "../../templates/templatesmanager.h"
#include "../../tools/commontools.h"

// Qt
#include <QMessageBox>

//! KDE
#include <KLocalizedString>
#include <KIO/OpenFileManagerWindowJob>

namespace Latte {
namespace Settings {
namespace Handler {

ViewsHandler::ViewsHandler(Dialog::ViewsDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui())
{
    m_viewsController = new Settings::Controller::Views(this);

    init();

    m_ui->cutBtn->setVisible(false);
    m_ui->copyBtn->setVisible(false);
    m_ui->pasteBtn->setVisible(false);
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

    //! New Button
    m_newViewAction = new QAction(i18nc("new view", "&New"), this);
    m_newViewAction->setToolTip(i18n("New dock or panel"));
    m_newViewAction->setIcon(QIcon::fromTheme("add"));
    m_newViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    connectActionWithButton(m_ui->newBtn, m_newViewAction);
    connect(m_newViewAction, &QAction::triggered, m_ui->newBtn, &QPushButton::showMenu);

    initViewTemplatesSubMenu();
    m_newViewAction->setMenu(m_viewTemplatesSubMenu);
    m_ui->newBtn->setMenu(m_viewTemplatesSubMenu);

    connect(corona()->templatesManager(), &Latte::Templates::Manager::viewTemplatesChanged, this, &ViewsHandler::initViewTemplatesSubMenu);

    //! Remove Button
    m_removeViewAction = new QAction(i18nc("remove layout", "Remove"), this);
    m_removeViewAction->setToolTip(i18n("Remove selected view"));
    m_removeViewAction->setIcon(QIcon::fromTheme("delete"));
    m_removeViewAction->setShortcut(QKeySequence(Qt::Key_Delete));
    connectActionWithButton(m_ui->removeBtn, m_removeViewAction);
    connect(m_removeViewAction, &QAction::triggered, this, &ViewsHandler::removeSelectedView);

    //! signals
    connect(this, &ViewsHandler::currentLayoutChanged, this, &ViewsHandler::reload);

    reload();

    emit currentLayoutChanged();

    //! connect layout combobox after the selected layout has been loaded
    connect(m_ui->layoutsCmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewsHandler::onCurrentLayoutIndexChanged);

    //!
    connect(m_viewsController, &Settings::Controller::Views::dataChanged, this, &ViewsHandler::dataChanged);
}

void ViewsHandler::initViewTemplatesSubMenu()
{
    if (!m_viewTemplatesSubMenu) {
        m_viewTemplatesSubMenu = new QMenu(m_ui->newBtn);
        m_viewTemplatesSubMenu->setMinimumWidth(m_ui->newBtn->width() * 2);
    } else {
        m_viewTemplatesSubMenu->clear();
    }

    /*Add View Templates for New Action*/
    Data::GenericBasicTable templates = corona()->templatesManager()->viewTemplates();

    bool customtemplateseparatoradded{false};

    for (int i=0; i<templates.rowCount(); ++i) {
        if (!customtemplateseparatoradded && templates[i].id.startsWith(QDir::homePath())) {
            m_viewTemplatesSubMenu->addSeparator();
            customtemplateseparatoradded = true;
        }

        QAction *newview = m_viewTemplatesSubMenu->addAction(templates[i].name);
        newview->setIcon(QIcon::fromTheme("document-new"));

        Data::Generic templateData = templates[i];

        connect(newview, &QAction::triggered, this, [&, templateData]() {
            newView(templateData);
        });
    }

    if (templates.rowCount() > 0) {
        QAction *openTemplatesDirectory = m_viewTemplatesSubMenu->addAction(i18n("Templates..."));
        openTemplatesDirectory->setToolTip(i18n("Open templates directory"));
        openTemplatesDirectory->setIcon(QIcon::fromTheme("edit"));

        connect(openTemplatesDirectory, &QAction::triggered, this, [&]() {
            KIO::highlightInFileManager({QString(Latte::configPath() + "/latte/templates/Dock.view.latte")});
        });
    }
}

void ViewsHandler::reload()
{
    o_data = m_dialog->layoutsController()->selectedLayoutCurrentData();
    o_data.views = m_dialog->layoutsController()->selectedLayoutViews();

    Latte::Data::LayoutIcon icon = m_dialog->layoutsController()->selectedLayoutIcon();

    m_ui->layoutsCmb->setCurrentText(o_data.name);
    m_ui->layoutsCmb->setLayoutIcon(icon);

    loadLayout(o_data);
}

Latte::Corona *ViewsHandler::corona() const
{
    return m_dialog->corona();
}

Ui::ViewsDialog *ViewsHandler::ui() const
{
    return m_ui;
}

void ViewsHandler::loadLayout(const Latte::Data::Layout &data)
{
    updateWindowTitle();
}

Latte::Data::Layout ViewsHandler::currentData() const
{
    return o_data;
}

Latte::Data::Layout ViewsHandler::originalData() const
{
    return m_dialog->layoutsController()->selectedLayoutOriginalData();
}

bool ViewsHandler::hasChangedData() const
{
    return m_viewsController->hasChangedData();
}

bool ViewsHandler::inDefaultValues() const
{
    //nothing special
    return true;
}

bool ViewsHandler::isSelectedLayoutOriginal() const
{
    return m_dialog->layoutsController()->isSelectedLayoutOriginal();
}

void ViewsHandler::reset()
{
    m_viewsController->reset();
}

void ViewsHandler::resetDefaults()
{
    //do nothing
}

void ViewsHandler::save()
{
    m_viewsController->save();
  //  m_dialog->layoutsController()->setLayoutProperties(currentData());
}


void ViewsHandler::newView(const Data::Generic &templateData)
{
    Data::ViewsTable views = Latte::Layouts::Storage::self()->views(templateData.id);

    if (views.rowCount() > 0) {
        Data::View viewfromtemplate = views[0];
        viewfromtemplate.setState(Data::View::OriginFromViewTemplate, templateData.id);
        viewfromtemplate.name = templateData.name;
        Data::View newview = m_viewsController->appendViewFromViewTemplate(viewfromtemplate);

        showInlineMessage(i18nc("settings:dock/panel added successfully","<b>%0</b> added successfully...").arg(newview.name),
                          KMessageWidget::Information);
    }
}

void ViewsHandler::removeSelectedView()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_removeViewAction->isEnabled()) {
        return;
    }

    if (!m_viewsController->hasSelectedView()) {
        return;
    }

    Latte::Data::View selectedView = m_viewsController->selectedViewCurrentData();
    m_viewsController->removeSelected();
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
        m_ui->layoutsCmb->setCurrentText(o_data.name);
    }
}

void ViewsHandler::updateWindowTitle()
{
    m_dialog->setWindowTitle(i18nc("<layout name> Docks/Panels","%0 Docks/Panels").arg(m_ui->layoutsCmb->currentText()));
}

int ViewsHandler::saveChanges()
{
    if (hasChangedData()) {
        QString layoutName = o_data.name;
        QString saveChangesText = i18n("The settings of <b>%0</b> layout have changed. Do you want to apply the changes or discard them?").arg(layoutName);

        return m_dialog->saveChangesConfirmation(saveChangesText);
    }

    return QMessageBox::Cancel;
}

}
}
}
