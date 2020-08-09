/*
*  Copyright 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "backgrounddelegate.h"

// local
#include "../models/layoutsmodel.h"
#include "../tools/settingstools.h"

// Qt
#include <QDebug>
#include <QModelIndex>
#include <QPainter>
#include <QString>


namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

const int MARGIN = 2;

BackgroundDelegate::BackgroundDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void BackgroundDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);

    //! draw underlying background
    QStyledItemDelegate::paint(painter, myOptions, index.model()->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

    QList<Latte::Data::LayoutIcon> icons = index.data(Qt::UserRole).value<QList<Latte::Data::LayoutIcon>>();

    if (icons.count() > 0) {
        int localMargin = MARGIN-1;// icons[0].isBackgroundFile && icons.count() == 1 ? qMin(option.rect.height()/4,MARGIN+5) : MARGIN-1;

        int aY = option.rect.y() + localMargin;
        int thick = option.rect.height() - localMargin*2;

        int centerX = option.rect.x() + (option.rect.width() / 2);
        int step = thick;
        int total_icons_width = (thick-step) + icons.count() * step;

        if (total_icons_width > option.rect.width()){
            step = thick/2;
            total_icons_width = (thick-step) + icons.count() * step;
        }

        int startX = centerX - (total_icons_width/2);

        for (int i=0; i<icons.count(); ++i) {
            int tX = startX + (i * step);
            drawIcon(painter, option, QRect(tX, aY, thick, thick), icons[i]);
        }
    }
}

void BackgroundDelegate::drawIcon(QPainter *painter, const QStyleOptionViewItem &option, const QRect &target, const Latte::Data::LayoutIcon &icon) const
{
    bool active = Latte::isActive(option);
    bool selected = Latte::isSelected(option);
    bool focused = Latte::isFocused(option);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (icon.isBackgroundFile) {
        int backImageMargin = qMin(option.rect.height()/4, MARGIN+2);
        QRect backTarget(target.x() + backImageMargin, target.y() + backImageMargin, target.width() - 2*backImageMargin, target.height() - 2*backImageMargin);

        QPixmap backImage(icon.name);
        backImage = backImage.copy(backTarget);

        QPalette::ColorRole textColorRole = selected ? QPalette::HighlightedText : QPalette::Text;

        QBrush imageBrush(backImage);
        QPen pen; pen.setWidth(1);
        pen.setColor(option.palette.color(Latte::colorGroup(option), textColorRole));

        painter->setBrush(imageBrush);
        painter->setPen(pen);

        painter->drawEllipse(backTarget);
    } else {
        QIcon::Mode mode = ((active && (selected || focused)) ? QIcon::Selected : QIcon::Normal);

        painter->drawPixmap(target, QIcon::fromTheme(icon.name).pixmap(target.height(), target.height(), mode));
    }

}

}
}
}
}

