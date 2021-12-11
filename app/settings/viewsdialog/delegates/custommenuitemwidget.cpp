/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "custommenuitemwidget.h"

// local
#include "../../generic/generictools.h"
#include "../../generic/genericviewtools.h"

// Qt
#include <QApplication>
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
   contentSize.setWidth(contentSize.width() + 1.5 * contentSize.height());
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

    bool inScreensColumn = !m_view.isValid();

    if (rect().contains(mapFromGlobal(QCursor::pos()))) {
        opt.state |= QStyle::State_Selected;
    }

    Latte::drawBackground(&painter, style(), opt);

    //! radio button
    int radiosize = opt.rect.height();
    QRect remained;

    if (qApp->layoutDirection() == Qt::LeftToRight) {
        remained = QRect(opt.rect.x() + radiosize , opt.rect.y(), opt.rect.width() - radiosize, opt.rect.height());
    } else {
        remained = QRect(opt.rect.x() , opt.rect.y(), opt.rect.width() - radiosize, opt.rect.height());
    }

    opt.rect = remained;

    if (!m_screen.id.isEmpty()) {
        int maxiconsize = 26;
        remained = Latte::remainedFromScreenDrawing(opt, m_screen.isScreensGroup(), maxiconsize);
        QRect availableScreenRect = Latte::drawScreen(&painter, opt, m_screen.isScreensGroup(), m_screen.geometry, maxiconsize);

        if (!m_view.id.isEmpty()) {
            Latte::drawView(&painter, opt, m_view, availableScreenRect);
        }
    }

    opt.rect = remained;

    //! text
    opt.text = opt.text.remove("&");
    if (qApp->layoutDirection() == Qt::LeftToRight) {
        //! add spacing
        remained = QRect(opt.rect.x() + 2 , opt.rect.y(), opt.rect.width() - 2, opt.rect.height());
    } else {
        //! add spacing
        remained = QRect(opt.rect.x() , opt.rect.y(), opt.rect.width() - 2, opt.rect.height());
    }

    opt.rect = remained;

    if (m_screen.isActive && inScreensColumn) {
        opt.text = "<b>" + opt.text + "</b>";
    }

    //style()->drawControl(QStyle::CE_MenuItem, &opt, &painter, this);
    Latte::drawFormattedText(&painter, opt);

    painter.restore();
}

}
}
}
}
