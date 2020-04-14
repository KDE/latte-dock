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


#include "detailshandler.h"

#include "../dialogs/detailsdialog.h"

namespace Latte {
namespace Settings {
namespace Handler {

DetailsHandler::DetailsHandler(Dialog::DetailsDialog *parentDialog)
    : Generic(parentDialog),
      m_parentDialog(parentDialog),
      m_ui(m_parentDialog->ui()),
      m_infoHandler(new DetailsInfoHandler(this, parentDialog))
{
}

DetailsHandler::~DetailsHandler()
{
}

bool DetailsHandler::dataAreChanged() const
{
    return m_infoHandler->dataAreChanged();
}

bool DetailsHandler::inDefaultValues() const
{
    //nothing special
    return true;
}


void DetailsHandler::reset()
{
}

void DetailsHandler::resetDefaults()
{
    //do nothing
}

void DetailsHandler::save()
{
}

void DetailsHandler::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval, QList<QAction *> actions)
{
}

}
}
}
