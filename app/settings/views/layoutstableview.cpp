/*
*  Copyright 2020 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "layoutstableview.h"

//! Qt
#include <QDebug>

//! KDE
#include <KLocalizedString>

#define MARGIN 15

namespace Latte {
namespace Settings {
namespace View {

LayoutsTableView::LayoutsTableView(QWidget *parent)
    : QTableView(parent)
{
    setAcceptDrops(true);

    m_overlayDropMessage = new QLabel(this);
    m_overlayDropMessage->setAcceptDrops(true);
    m_overlayDropMessage->setVisible(false);
    m_overlayDropMessage->setText(i18n("Drop layout files here..."));

    m_overlayDropMessage->setAutoFillBackground(true);
    QFont fn = m_overlayDropMessage->font();
    fn.setBold(true);
    fn.setPointSize(fn.pointSize() * 3);
    m_overlayDropMessage->setFont(fn);
    m_overlayDropMessage->setAlignment(Qt::AlignCenter);

    QPalette palette = m_overlayDropMessage->palette();
    QColor backgroundColor = palette.color(QPalette::Background);
    QColor foregroundColor = palette.color(QPalette::Foreground);

    QColor borderColor = foregroundColor;
    borderColor.setAlphaF(0.5);
    backgroundColor.setAlphaF(0.8);
    foregroundColor.setAlphaF(0.5);

    qDebug() << borderColor.name() << " _ " << backgroundColor.name();

    QString css = "QLabel{border: 1px solid "+borderColor.name(QColor::HexArgb)+"; border-radius: 15px;";
    css += "background:"+backgroundColor.name(QColor::HexArgb)+"; color:" +foregroundColor.name(QColor::HexArgb)+ ";}";

    m_overlayDropMessage->setStyleSheet(css);
}

void LayoutsTableView::paintEvent(QPaintEvent *event)
{
    QTableView::paintEvent(event);
}

void LayoutsTableView::dragEntered()
{
    m_overlayDropMessage->move(MARGIN, MARGIN);
    m_overlayDropMessage->resize(width() - 2*MARGIN, height() - 2*MARGIN);

    m_overlayDropMessage->raise();
    m_overlayDropMessage->setVisible(true);
}

void LayoutsTableView::dragLeft()
{
    m_overlayDropMessage->setVisible(false);
}

}
}
}

