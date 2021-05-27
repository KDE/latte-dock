/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PERSISTENTMENU_H
#define PERSISTENTMENU_H

//Qt
#include <QMenu>
#include <QMouseEvent>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

class PersistentMenu : public QMenu
{
  Q_OBJECT
public:
  PersistentMenu(QWidget *parent = nullptr);

  int masterIndex() const;
  void setMasterIndex(const int &index);

protected:
  void setVisible(bool visible) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

signals:
  void masterIndexChanged(const int &masterRow);

private:
  bool m_blockHide{false};

  int m_masterIndex{-1};

};

}
}
}
}

#endif
