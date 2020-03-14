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
        QString name = index.data(Qt::DisplayRole).toString();
        lineEditor->setText(name);
    }
}

void LayoutName::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *lineEditor = qobject_cast<QLineEdit *>(editor);

    if (lineEditor) {
        model->setData(index, lineEditor->text(), Qt::DisplayRole);
    }
}

void LayoutName::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool isLocked = index.data(Model::Layouts::LAYOUTISLOCKEDROLE).toBool();
    bool isShared = index.data(Model::Layouts::LAYOUTISSHAREDROLE).toBool() && index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();
    bool isActive = index.data(Model::Layouts::LAYOUTISACTIVEROLE).toBool();
    bool isChanged = index.data(Model::Layouts::LAYOUTNAMEWASEDITEDROLE).toBool();

    bool showTwoIcons = isLocked && isShared;

    QStyleOptionViewItem adjustedOption = option;
    //! Remove the focus dotted lines
    adjustedOption.state = (adjustedOption.state & ~QStyle::State_HasFocus);
    adjustedOption.displayAlignment = Qt::AlignHCenter;

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

        myOptionMain.font.setBold(isActive);

        myOptionS.rect = destinationS;
        myOptionE.rect = destinationE;
        myOptionMain.rect.setX(option.rect.x() + startWidth);
        myOptionMain.rect.setWidth(option.rect.width() - startWidth - endWidth);

        QStyledItemDelegate::paint(painter, myOptionMain, index);

        //! draw background at edges
        QStyledItemDelegate::paint(painter, myOptionS, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

        QStyledItemDelegate::paint(painter, myOptionE, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

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

    adjustedOption.font.setBold(isActive);
    adjustedOption.font.setItalic(isChanged);

    QStyledItemDelegate::paint(painter, adjustedOption, index);
}

}
}
}
}
