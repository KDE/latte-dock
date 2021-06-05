/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "screenshandler.h"

// local
#include <coretypes.h>
#include "ui_screensdialog.h"
#include "screensdialog.h"
#include "screensmodel.h"
#include "delegates/checkboxdelegate.h"
#include "../../lattecorona.h"
#include "../../screenpool.h"

// KDE
#include <KMessageBox>

namespace Latte {
namespace Settings {
namespace Handler {

ScreensHandler::ScreensHandler(Dialog::ScreensDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui()),
      m_screensModel(new Model::Screens(this))
{
    init();
}

ScreensHandler::~ScreensHandler()
{
}

void ScreensHandler::init()
{
    m_ui->screensTable->horizontalHeader()->setSectionsClickable(false);
    m_ui->screensTable->verticalHeader()->setVisible(false);

    //! Data Changed
    connect(m_screensModel, &Settings::Model::Screens::screenDataChanged, this, &ScreensHandler::dataChanged);

    //! Screens Proxy Model
    m_screensProxyModel = new QSortFilterProxyModel(this);
    m_screensProxyModel->setSourceModel(m_screensModel);

    m_ui->screensTable->setModel(m_screensProxyModel);

    m_screensProxyModel->setSortRole(Model::Screens::SORTINGROLE);
    m_screensProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_ui->screensTable->sortByColumn(Model::Screens::SCREENCOLUMN, Qt::DescendingOrder);

    //! Delegates
    m_ui->screensTable->setItemDelegateForColumn(Model::Screens::SCREENCOLUMN, new Settings::Screens::Delegate::CheckBox(this));

    //! Buttons
    connect(m_ui->deselectAllBtn, &QPushButton::clicked, this, &ScreensHandler::deselectAll);

    //! load data
    m_screensModel->setData(m_dialog->layoutsController()->screensData());

    //! signals
    connect(m_screensModel, &Settings::Model::Screens::screenDataChanged, this, &ScreensHandler::onScreenDataChanged);

    onScreenDataChanged();
}

bool ScreensHandler::hasChangedData() const
{
    return m_screensModel->hasChangedData();
}

bool ScreensHandler::inDefaultValues() const
{
    return m_screensModel->inDefaultValues();
}

void ScreensHandler::deselectAll()
{
    m_screensModel->deselectAll();
}

void ScreensHandler::reset()
{
    if (m_screensModel->hasChangedData()) {
        m_screensModel->reset();
    }
}

void ScreensHandler::resetDefaults()
{
    reset();
}

void ScreensHandler::onScreenDataChanged()
{
    m_dialog->removeNowButton()->setEnabled(m_screensModel->hasSelected());
}


void ScreensHandler::save()
{
    //do nothing
}

/*bool ScreensHandler::overwriteConfirmation(const QString &fileName)
{
    return (KMessageBox::warningYesNo(m_dialog,
                                      i18n("The file \"%1\" already exists. Do you wish to overwrite it?", fileName),
                                      i18n("Overwrite File?"),
                                      KStandardGuiItem::overwrite(),
                                      KStandardGuiItem::cancel()) == KMessageBox::Yes);
}*/

}
}
}
