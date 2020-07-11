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

#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

// Qt
#include <QEvent>
#include <QMenu>
#include <QMetaMethod>
#include <QQuickItem>
#include <QQuickView>
#include <QMouseEvent>
#include <QObject>
#include <QPointer>

namespace Plasma {
class Applet;
class Containment;
}

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {

class ContextMenu : public QObject
{
    Q_OBJECT

public:
    ContextMenu(Latte::View *view);
    ~ContextMenu() override;

    QMenu *menu();

    bool mousePressEvent(QMouseEvent *event);
    bool mousePressEventForContainmentMenu(QQuickView *view, QMouseEvent *event);

signals:
    void menuChanged();

private slots:
    void menuAboutToHide();

private:
    void addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event);
    void addContainmentActions(QMenu *desktopMenu, QEvent *event);
    void updateAppletContainsMethod();

    QPoint popUpRelevantToParent(const QRect &parentItem, const QRect popUpRect);
    QPoint popUpRelevantToGlobalPoint(const QRect &parentItem, const QRect popUpRect);

    QPoint popUpTopLeft(Plasma::Applet *applet, const QRect popUpRect);

    Plasma::Containment *containmentById(uint id);


private:
    QPointer<QMenu> m_contextMenu;
    QMetaMethod m_appletContainsMethod;
    QQuickItem *m_appletContainsMethodItem{nullptr};

    Latte::View *m_latteView;

    friend class Latte::View;
};

}
}

#endif // DOCKMENUMANAGER_H
