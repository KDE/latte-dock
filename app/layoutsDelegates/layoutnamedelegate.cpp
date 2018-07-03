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

#include "layoutnamedelegate.h"

#include <QApplication>
#include <QBitmap>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>

const int HIDDENTEXTCOLUMN = 1;

LayoutNameDelegate::LayoutNameDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void LayoutNameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool isLocked = index.data(Qt::UserRole).toBool();

    if (isLocked) {
        QStandardItemModel *model = (QStandardItemModel *) index.model();
        QString nameText = index.data(Qt::DisplayRole).toString();

        //! font metrics
        QFontMetrics fm(option.font);
        int textWidth = fm.width(nameText);
        int thick = option.rect.height();
        int startWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? thick : qBound(0, option.rect.width() - textWidth - thick, thick);
        int endWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? qBound(0, option.rect.width() - textWidth - thick, thick) : thick;

        QRect destinationS(option.rect.x(), option.rect.y(), startWidth, thick);
        QRect destinationE(option.rect.x() + option.rect.width() - thick, option.rect.y(), endWidth, thick);

        QStyleOptionViewItem myOptionS = option;
        QStyleOptionViewItem myOptionE = option;
        QStyleOptionViewItem myOptionMain = option;
        myOptionS.rect = destinationS;
        myOptionE.rect = destinationE;
        myOptionMain.rect.setX(option.rect.x() + startWidth);
        myOptionMain.rect.setWidth(option.rect.width() - startWidth - endWidth);

        QStyledItemDelegate::paint(painter, myOptionMain, index);

        //! draw background at edges
        QStyledItemDelegate::paint(painter, myOptionS, model->index(index.row(), HIDDENTEXTCOLUMN));

        //! Lock Icon 1: Unicode character attempt
        /*QString s = QChar(0x2318);
          QString s = QChar(0x2757);
          myOptionE.text = s;*/
        QStyledItemDelegate::paint(painter, myOptionE, model->index(index.row(), HIDDENTEXTCOLUMN));

        //! Lock Icon 2: QIcon attempt that doesnt change color
        QIcon lockIcon = QIcon::fromTheme("object-locked");

        if (qApp->layoutDirection() == Qt::RightToLeft) {
            painter->drawPixmap(destinationS, lockIcon.pixmap(thick, thick));
        } else {
            painter->drawPixmap(destinationE, lockIcon.pixmap(thick, thick));
        }

        //! Lock Icon 3: QIcon and change colors attempt
        /*QIcon lockIcon = QIcon::fromTheme("object-locked");
        QPixmap origPixmap = lockIcon.pixmap(thick, thick);
        QPixmap lockPixmap = origPixmap;

        QBrush nBrush;

        if ((option.state & QStyle::State_Active) && (option.state & QStyle::State_Selected)) {
            nBrush = option.palette.brush(QPalette::Active, QPalette::HighlightedText);
        } else {
            nBrush = option.palette.brush(QPalette::Inactive, QPalette::Text);
        }

        lockPixmap.fill(nBrush.color());
        lockPixmap.setMask(origPixmap.createMaskFromColor(Qt::transparent));

        painter->drawPixmap(destinationE, lockPixmap);*/

        //! Lock Icon 4: Plasma::Svg and change color group attempt
        /*Plasma::Svg svgIcon;

        svgIcon.setStatus(Plasma::Svg::Normal);
        svgIcon.setUsingRenderingCache(false);
        svgIcon.setDevicePixelRatio(qApp->devicePixelRatio());

        //! find path
        //try to load from iconloader an svg with Plasma::Svg
        const auto *iconTheme = KIconLoader::global()->theme();
        QString iconPath;

        if (iconTheme) {
            iconPath = iconTheme->iconPath("object-locked" + QLatin1String(".svg")
                                           , thick
                                           , KIconLoader::MatchBest);

            if (iconPath.isEmpty()) {
                iconPath = iconTheme->iconPath("object-locked" + QLatin1String(".svgz")
                                               , thick
                                               , KIconLoader::MatchBest);
            }
        } else {
            qWarning() << "KIconLoader has no theme set";
        }

        if (!iconPath.isEmpty()) {
            svgIcon.setImagePath(iconPath);

            if ((option.state & QStyle::State_Active) && (option.state & QStyle::State_Selected)) {
                svgIcon.setColorGroup(Plasma::Theme::ComplementaryColorGroup);
            } else {
                svgIcon.setColorGroup(Plasma::Theme::NormalColorGroup);
            }

            svgIcon.resize(thick, thick);
        }

        painter->drawPixmap(destinationE, svgIcon.pixmap());*/

        return;
    }

    QStyledItemDelegate::paint(painter, option, index);
}

