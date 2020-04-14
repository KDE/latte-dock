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
#include "../dialogs/detailsdialog.h"
#include "../widgets/patternwidget.h"
#include "../../layout/abstractlayout.h"

namespace Latte {
namespace Settings {
namespace Handler {

DetailsInfoHandler::DetailsInfoHandler(QObject *parent, Dialog::DetailsDialog *parentDialog)
    : Generic(parent),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui())
{
    init();
}

DetailsInfoHandler::~DetailsInfoHandler()
{
}

void DetailsInfoHandler::init()
{
    Settings::Data::Layout selectedLayoutCurrent = m_parentDialog->layoutsController()->selectedLayoutCurrentData();
    initLayout(selectedLayoutCurrent);
}

void DetailsInfoHandler::initLayout(const Data::Layout &data)
{
    o_data = data;
    c_data = data;

    m_ui->colorPatternWidget->setBackground(m_parentDialog->layoutsController()->colorPath(o_data.color));
    m_ui->backPatternWidget->setBackground(o_data.background);

    m_ui->colorPatternWidget->setTextColor(Layout::AbstractLayout::defaultTextColor(o_data.color));
    m_ui->backPatternWidget->setTextColor(o_data.textColor);
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

void DetailsInfoHandler::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval, QList<QAction *> actions)
{
}

}
}
}
