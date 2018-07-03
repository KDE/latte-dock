/*
*  Copyright 2018 Michail Vourlakos <mvourlakos@gmail.com>
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


#ifndef DOCKMENUMANAGER_H
#define DOCKMENUMANAGER_H

#include <QEvent>
#include <QMenu>
#include <QMetaMethod>
#include <QQuickItem>
#include <QMouseEvent>
#include <QObject>

namespace Plasma {
class Applet;
class Containment;
}

namespace Latte {
class DockView;
}

namespace Latte {

class DockMenuManager : public QObject
{
    Q_OBJECT

public:
    DockMenuManager(DockView *view);
    ~DockMenuManager() override;

    QMenu *contextMenu();

    bool mousePressEvent(QMouseEvent *event);

signals:
    void contextMenuChanged();

private slots:
    void menuAboutToHide();

private:
    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);
    void updateAppletContainsMethod();

    Plasma::Containment *containmentById(uint id);

private:
    QMenu *m_contextMenu{nullptr};
    QMetaMethod m_appletContainsMethod;
    QQuickItem *m_appletContainsMethodItem{nullptr};

    DockView *m_dockView;

    friend class DockView;
};

}

#endif // DOCKMENUMANAGER_H
