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

#ifndef PERSISTENTMENU_H
#define PERSISTENTMENU_H

//Qt
#include <QMenu>
#include <QMouseEvent>

class PersistentMenu : public QMenu
{
  Q_OBJECT
public:
  PersistentMenu(QWidget *parent = nullptr);

protected:
  void setVisible (bool visible) override;
  void mouseReleaseEvent (QMouseEvent *e) override;

private:
  bool m_blockHide{false};

};

#endif
