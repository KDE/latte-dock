/*
*  Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "persistentmenu.h"

namespace Latte {
namespace Settings {
namespace Layouts {
namespace Delegates {

PersistentMenu::PersistentMenu(QWidget *parent)
    : QMenu (parent),
      m_blockHide(false)
{
}

void PersistentMenu::setVisible (bool visible)
{
  if (m_blockHide) {
      m_blockHide = false;
      return;
  }

  QMenu::setVisible (visible);
}

int PersistentMenu::masterIndex() const
{
    return m_masterIndex;
}

void PersistentMenu::setMasterIndex(const int &index)
{
    if (m_masterIndex == index) {
        return;
    }

    m_masterIndex = index;
    emit masterIndexChanged(index);
}

void PersistentMenu::mouseReleaseEvent (QMouseEvent *e)
{
  const QAction *action = actionAt (e->pos ());
  if (action) {
      m_blockHide = true;
  }

  QMenu::mouseReleaseEvent (e);
}

}
}
}
}
