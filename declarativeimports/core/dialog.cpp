/*
 *  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "dialog.h"

// Qt
#include <QScreen>
#include <QWindow>


namespace Latte {
namespace Quick {

Dialog::Dialog(QQuickItem *parent)
    : PlasmaQuick::Dialog(parent)
{
}

bool Dialog::containsMouse() const
{
    return m_containsMouse;
}

void Dialog::setContainsMouse(bool contains)
{
    if (m_containsMouse == contains) {
        return;
    }

    m_containsMouse = contains;
    emit containsMouseChanged();
}

Plasma::Types::Location Dialog::edge() const
{
    return m_edge;
}

void Dialog::setEdge(const Plasma::Types::Location &edge)
{
    if (m_edge == edge) {
        return;
    }

    m_edge = edge;
    emit edgeChanged();
}

bool Dialog::isRespectingAppletsLayoutGeometry() const
{
    //! As it appears plasma applets popups are defining their popups to Normal window.
    return (type() == Dialog::Normal || type() == Dialog::PopupMenu);
}

QRect Dialog::appletsLayoutGeometryFromContainment() const
{
    QVariant geom = visualParent() && visualParent()->window() ? visualParent()->window()->property("_applets_layout_geometry") : QVariant();
    return geom.isValid() ? geom.toRect() : QRect();
}

int Dialog::appletsPopUpMargin() const
{
    QVariant margin = visualParent() && visualParent()->window() ? visualParent()->window()->property("_applets_popup_margin") : QVariant();
    return margin.isValid() ? margin.toInt() : -1;
}

void Dialog::updatePopUpEnabledBorders()
{
    QRect appletslayoutgeometry = appletsLayoutGeometryFromContainment();
    int appletspopupmargin = appletsPopUpMargin();

    //! Plasma Scenario
    bool hideEdgeBorder = isRespectingAppletsLayoutGeometry() && !appletslayoutgeometry.isEmpty() && appletspopupmargin==-1;

    if (hideEdgeBorder) {
        setLocation(m_edge);
    } else {
        setLocation(Plasma::Types::Floating);
    }
}

void Dialog::adjustGeometry(const QRect &geom)
{
    auto visualparent = visualParent();

    if (visualparent && visualparent->window() && visualparent->window()->screen()) {
        updatePopUpEnabledBorders();

        QPointF parenttopleftf = visualparent->mapToGlobal(QPointF(0, 0));
        QPoint parenttopleft = parenttopleftf.toPoint();
        QScreen *screen = visualparent->window()->screen();
        QRect screengeometry = screen->geometry();

        int x = 0;
        int y = 0;

        if (m_edge == Plasma::Types::LeftEdge || m_edge == Plasma::Types::RightEdge) {
            y = parenttopleft.y() + (visualparent->height()/2) - (geom.height()/2);
        } else {
            x = parenttopleft.x() + (visualparent->width()/2) - (geom.width()/2);
        }

        int popupmargin = qMax(0, appletsPopUpMargin());

        if (m_edge == Plasma::Types::LeftEdge) {
            x = parenttopleft.x() + visualparent->width() + popupmargin;
        } else if (m_edge == Plasma::Types::RightEdge) {
            x = parenttopleft.x() - geom.width() - popupmargin;
        } else if (m_edge == Plasma::Types::TopEdge) {
            y = parenttopleft.y() + visualparent->height() + popupmargin;
        } else { // bottom case
            y = parenttopleft.y() - geom.height() - popupmargin;
        }

        x = qBound(screengeometry.x(), x, screengeometry.right()-1);
        y = qBound(screengeometry.y(), y, screengeometry.bottom()-1);

        QRect appletslayoutgeometry = appletsLayoutGeometryFromContainment();

        if (isRespectingAppletsLayoutGeometry() && !appletslayoutgeometry.isEmpty()) {
            QPoint appletsglobaltopleft = visualparent->window()->mapToGlobal(appletslayoutgeometry.topLeft());

            QRect appletsglobalgeometry(appletsglobaltopleft.x(), appletsglobaltopleft.y(), appletslayoutgeometry.width(), appletslayoutgeometry.height());

            if (m_edge == Plasma::Types::LeftEdge || m_edge == Plasma::Types::RightEdge) {
                int bottomy = appletsglobalgeometry.bottom()-geom.height();

                if (appletsglobalgeometry.height() >= geom.height()) {
                    y = qBound(appletsglobalgeometry.y(), y, bottomy + 1);
                }
            } else {
                int rightx = appletsglobalgeometry.right()-geom.width();

                if (appletsglobalgeometry.width() >= geom.width()) {
                    x = qBound(appletsglobalgeometry.x(), x, rightx + 1);
                }
            }
        }

        QRect repositionedrect(x, y, geom.width(), geom.height());
        setGeometry(repositionedrect);
        return;
    }

    PlasmaQuick::Dialog::adjustGeometry(geom);
}


bool Dialog::event(QEvent *e)
{
    if (e->type() == QEvent::Enter) {
        setContainsMouse(true);
    } else if (e->type() == QEvent::Leave
               || e->type() == QEvent::Hide) {
        setContainsMouse(false);
    }

    return PlasmaQuick::Dialog::event(e);
}

}
}
