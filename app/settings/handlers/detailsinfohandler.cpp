/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "detailsinfohandler.h"

// local
#include "ui_detailsdialog.h"
#include "detailshandler.h"
#include "../dialogs/detailsdialog.h"
#include "../widgets/patternwidget.h"
#include "../../layout/abstractlayout.h"

namespace Latte {
namespace Settings {
namespace Handler {

DetailsInfoHandler::DetailsInfoHandler(Dialog::DetailsDialog *parentDialog, DetailsHandler *parentHandler)
    : Generic(parentDialog, parentHandler),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui()),
      m_parentHandler(parentHandler)
{
    init();
}

DetailsInfoHandler::~DetailsInfoHandler()
{
}

void DetailsInfoHandler::init()
{
    initLayout(m_parentHandler->currentData());
}

void DetailsInfoHandler::initLayout(const Data::Layout &data)
{
    m_ui->colorPatternWidget->setBackground(m_parentDialog->layoutsController()->colorPath(data.color));
    m_ui->backPatternWidget->setBackground(data.background);

    m_ui->colorPatternWidget->setTextColor(Layout::AbstractLayout::defaultTextColor(data.color));
    m_ui->backPatternWidget->setTextColor(data.textColor);
}

bool DetailsInfoHandler::dataAreChanged() const
{
    return true;
}

bool DetailsInfoHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void DetailsInfoHandler::reset()
{
}

void DetailsInfoHandler::resetDefaults()
{
    //do nothing
}

void DetailsInfoHandler::save()
{
}

}
}
}
