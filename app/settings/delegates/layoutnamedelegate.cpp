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
#include "../models/layoutsmodel.h"
// local
#include "../tools/settingstools.h"

// Qt
#include <QApplication>
#include <QBitmap>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStandardItemModel>

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

LayoutName::LayoutName(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *LayoutName::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void LayoutName::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editor);

    if (lineEditor) {
        QString name = index.data(Qt::UserRole).toString();
        lineEditor->setText(name);
    }
}

void LayoutName::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editor);

    if (lineEditor) {
        model->setData(index, lineEditor->text(), Qt::UserRole);
    }
}

void LayoutName::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool inMultiple = index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();

    bool isLocked = index.data(Model::Layouts::ISLOCKEDROLE).toBool();
    bool isShared = inMultiple && index.data(Model::Layouts::ISSHAREDROLE).toBool();
    bool isActive = index.data(Model::Layouts::ISACTIVEROLE).toBool();

    bool isNewLayout = index.data(Model::Layouts::ISNEWLAYOUTROLE).toBool();
    bool hasChanges = index.data(Model::Layouts::LAYOUTHASCHANGESROLE).toBool();

    QString name = index.data(Qt::UserRole).toString();

    bool isChanged = (isNewLayout || hasChanges);

    bool showTwoIcons = isLocked && isShared;

    QStyleOptionViewItem adjustedOption = option;

    //! Remove the focus dotted lines
    adjustedOption.state = (adjustedOption.state & ~QStyle::State_HasFocus);
    adjustedOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (isLocked || isShared) {
        QStandardItemModel *model = (QStandardItemModel *) index.model();


        bool active = Latte::isActive(option);
        bool enabled = Latte::isEnabled(option);
        bool selected = Latte::isSelected(option);
        bool focused = Latte::isFocused(option);
        bool hovered = Latte::isHovered(option);

        //! font metrics
        QFontMetrics fm(option.font);
        int textWidth = fm.boundingRect(name).width();
        int thick = option.rect.height();
        int length = showTwoIcons ? (2 * thick + 2) : thick;

        int startWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? length : 0;
        int endWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? 0 : length;

        QRect destinationS(option.rect.x(), option.rect.y(), startWidth, thick);
        QRect destinationE(option.rect.x() + option.rect.width() - endWidth, option.rect.y(), endWidth, thick);

        QStyleOptionViewItem myOptionS = adjustedOption;
        QStyleOptionViewItem myOptionE = adjustedOption;
        QStyleOptionViewItem myOptionMain = adjustedOption;

        myOptionMain.font.setBold(isActive);
        myOptionMain.font.setItalic(isChanged);

        myOptionS.rect = destinationS;
        myOptionE.rect = destinationE;
        myOptionMain.rect.moveLeft(option.rect.x() + startWidth);
        myOptionMain.rect.setWidth(option.rect.width() - startWidth - endWidth);

        QStyledItemDelegate::paint(painter, myOptionMain, index);

        //! draw background below icons
        //! HIDDENTEXTCOLUMN is just needed to draw empty background rectangles
        QStyledItemDelegate::paint(painter, myOptionS, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));
        QStyledItemDelegate::paint(painter, myOptionE, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

        //! Lock Icon
        QIcon firstIcon = isLocked && !showTwoIcons ? QIcon::fromTheme("object-locked") : QIcon::fromTheme("document-share");

        QIcon::Mode mode = ((active && (selected || focused)) ? QIcon::Selected : QIcon::Normal);

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

    adjustedOption.font.setBold(isActive);
    adjustedOption.font.setItalic(isChanged);

    QStyledItemDelegate::paint(painter, adjustedOption, index);
}

}
}
}
}
