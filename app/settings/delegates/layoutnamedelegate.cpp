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

// local
#include "../settingsdialog.h"
#include "../tools/settingstools.h"

// Qt
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
    auto *settingsDialog = qobject_cast<Latte::SettingsDialog *>(parent);

    if (settingsDialog) {
        m_settingsDialog = settingsDialog;
    }
}

void LayoutNameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool isLocked = index.data(Qt::UserRole).toBool();
    bool isShared = m_settingsDialog->isShared(index.row()) && m_settingsDialog->inMultipleLayoutsLook();

    bool showTwoIcons = isLocked && isShared;

    QStyleOptionViewItem adjustedOption = option;
    //! Remove the focus dotted lines
    adjustedOption.state = (adjustedOption.state & ~QStyle::State_HasFocus);

    if (isLocked || isShared) {
        QStandardItemModel *model = (QStandardItemModel *) index.model();
        QString nameText = index.data(Qt::DisplayRole).toString();
        bool selected = Latte::isSelected(option);

        //! font metrics
        QFontMetrics fm(option.font);
        int textWidth = fm.boundingRect(nameText).width();
        int thick = option.rect.height();
        int length = showTwoIcons ? (2 * thick + 2) : thick;

        int startWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? length : qBound(0, option.rect.width() - textWidth - length, length);
        int endWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? qBound(0, option.rect.width() - textWidth - length, length) : length;

        QRect destinationS(option.rect.x(), option.rect.y(), startWidth, thick);
        QRect destinationE(option.rect.x() + option.rect.width() - endWidth, option.rect.y(), endWidth, thick);

        QStyleOptionViewItem myOptionS = adjustedOption;
        QStyleOptionViewItem myOptionE = adjustedOption;
        QStyleOptionViewItem myOptionMain = adjustedOption;
        myOptionS.rect = destinationS;
        myOptionE.rect = destinationE;
        myOptionMain.rect.setX(option.rect.x() + startWidth);
        myOptionMain.rect.setWidth(option.rect.width() - startWidth - endWidth);

        QStyledItemDelegate::paint(painter, myOptionMain, index);

        //! draw background at edges
        QStyledItemDelegate::paint(painter, myOptionS, model->index(index.row(), HIDDENTEXTCOLUMN));

        QStyledItemDelegate::paint(painter, myOptionE, model->index(index.row(), HIDDENTEXTCOLUMN));

        //! Lock Icon
        QIcon firstIcon = isLocked && !showTwoIcons ? QIcon::fromTheme("object-locked") : QIcon::fromTheme("document-share");
        QIcon::Mode mode = selected ? QIcon::Selected : QIcon::Normal;

        if (qApp->layoutDirection() == Qt::RightToLeft) {
            painter->drawPixmap(QRect(option.rect.x(), option.rect.y(), thick, thick), firstIcon.pixmap(thick, thick, mode));

            if (showTwoIcons) {
                QIcon secondIcon = QIcon::fromTheme("object-locked");
                painter->drawPixmap(QRect(option.rect.x() + thick + 2, option.rect.y(), thick, thick), secondIcon.pixmap(thick, thick, mode));
            }
        } else {
            painter->drawPixmap(QRect(option.rect.x() + option.rect.width() - endWidth, option.rect.y(), thick, thick), firstIcon.pixmap(thick, thick, mode));

            if (showTwoIcons) {
                QIcon secondIcon = QIcon::fromTheme("object-locked");
                painter->drawPixmap(QRect(option.rect.x() + option.rect.width() - thick, option.rect.y(), thick, thick), secondIcon.pixmap(thick, thick, mode));
            }
        }

        return;
    }

    QStyledItemDelegate::paint(painter, adjustedOption, index);
}

