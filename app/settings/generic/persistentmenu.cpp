/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "persistentmenu.h"

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

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

  QMenu::setVisible(visible);
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
  const QAction *action = actionAt(e->pos());
  if (action) {
      m_blockHide = true;
  }

  QMenu::mouseReleaseEvent (e);
}

}
}
}
}
