/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "custommenuitemwidget.h"

// local
#include "../../generic/generictools.h"

// Qt
#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QRadioButton>
#include <QStyleOptionMenuItem>

namespace Latte {
namespace Settings {
namespace View {
namespace Widget {

CustomMenuItemWidget::CustomMenuItemWidget(QAction* action, QWidget *parent)
    : QWidget(parent),
      m_action(action)
{
    QHBoxLayout *l = new QHBoxLayout;

    auto radiobtn = new QRadioButton(this);
    radiobtn->setCheckable(true);
    radiobtn->setChecked(action->isChecked());

    l->addWidget(radiobtn);
    setLayout(l);

    setMouseTracking(true);
}

void CustomMenuItemWidget::setScreen(const Latte::Data::Screen &screen)
{
    m_screen = screen;
}

void CustomMenuItemWidget::setView(const Latte::Data::View &view)
{
    m_view = view;
}

QSize CustomMenuItemWidget::minimumSizeHint() const
{
   QStyleOptionMenuItem opt;
   QSize contentSize = fontMetrics().size(Qt::TextSingleLine | Qt::TextShowMnemonic, m_action->text());
   contentSize.setHeight(contentSize.height() + 9);
   contentSize.setWidth(contentSize.width() + 4 * contentSize.height());
   return style()->sizeFromContents(QStyle::CT_MenuItem, &opt, contentSize, this);
}

void CustomMenuItemWidget::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.save();
    QStyleOptionMenuItem opt;
    opt.initFrom(this);
    opt.text = m_action->text();
    opt.menuItemType = QStyleOptionMenuItem::Normal;
    opt.menuHasCheckableItems = false;

    if (rect().contains(mapFromGlobal(QCursor::pos()))) {
        opt.state |= QStyle::State_Selected;
    }

    Latte::drawBackground(&painter, style(), opt);

    int radiosize = opt.rect.height() + 2;
    QRect remained = QRect(opt.rect.x() + radiosize , opt.rect.y(), opt.rect.width() - radiosize, opt.rect.height());

    opt.rect = remained;

    if (!m_screen.id.isEmpty()) {
        int maxiconsize = 26;
        remained = Latte::remainedFromScreenDrawing(opt, maxiconsize);
        QRect availableScreenRect = Latte::drawScreen(&painter, opt, m_screen.geometry, maxiconsize);

        if (!m_view.id.isEmpty()) {
            Latte::drawView(&painter, opt, m_view, availableScreenRect);
        }
    }

    opt.rect = remained;
    style()->drawControl(QStyle::CE_MenuItem, &opt, &painter, this);

    painter.restore();
}

}
}
}
}
