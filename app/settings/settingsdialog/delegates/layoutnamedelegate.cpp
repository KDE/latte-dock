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
#include "../layoutsmodel.h"
#include "../../generic/generictools.h"

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

const int INDICATORCHANGESLENGTH = 6;
const int INDICATORCHANGESMARGIN = 2;

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

void LayoutName::drawHasChangesIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //! draw changes circle indicator
    int csize{INDICATORCHANGESLENGTH};
    int tsize{INDICATORCHANGESLENGTH + INDICATORCHANGESMARGIN*2};

    //! Draw indicator background
    QStandardItemModel *model = (QStandardItemModel *) index.model();
    if (qApp->layoutDirection() == Qt::RightToLeft) {
        QStyleOptionViewItem indicatorOption = option;
        indicatorOption.rect = QRect(option.rect.x(), option.rect.y(), tsize, option.rect.height());
        QStyledItemDelegate::paint(painter, indicatorOption, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));
    } else {
        QStyleOptionViewItem indicatorOption = option;
        indicatorOption.rect = QRect(option.rect.x() + option.rect.width() - tsize, option.rect.y(), tsize, option.rect.height());
        QStyledItemDelegate::paint(painter, indicatorOption, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));
    }

    bool isNewLayout = index.data(Model::Layouts::ISNEWLAYOUTROLE).toBool();
    bool hasChanges = index.data(Model::Layouts::LAYOUTHASCHANGESROLE).toBool();
    bool isChanged = (isNewLayout || hasChanges);

    if (isChanged) {
        QRect changesRect = (qApp->layoutDirection() == Qt::RightToLeft) ? QRect(option.rect.x() + INDICATORCHANGESMARGIN, option.rect.y() + option.rect.height()/2 - csize/2, csize, csize) :
                                                                           QRect(option.rect.x() + option.rect.width() - csize - INDICATORCHANGESMARGIN, option.rect.y() + option.rect.height()/2 - csize/2, csize, csize);

        QColor plasmaOrange(246, 116, 0); //orangish color used from plasma systemsettings #f67400
        QBrush backBrush(plasmaOrange);
        QPen pen; pen.setWidth(1);
        pen.setColor(plasmaOrange);

        painter->setBrush(backBrush);
        painter->setPen(pen);
        painter->drawEllipse(changesRect);
    }
}


void LayoutName::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool inMultiple = index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();

    bool isLocked = index.data(Model::Layouts::ISLOCKEDROLE).toBool();
    bool isActive = index.data(Model::Layouts::ISACTIVEROLE).toBool();
    bool isConsideredActive = index.data(Model::Layouts::ISCONSIDEREDACTIVEROLE).toBool();

    bool isNewLayout = index.data(Model::Layouts::ISNEWLAYOUTROLE).toBool();
    bool hasChanges = index.data(Model::Layouts::LAYOUTHASCHANGESROLE).toBool();

    QString name = index.data(Qt::UserRole).toString();

    bool isChanged = (isNewLayout || hasChanges);
    bool drawTwoIcons = isLocked && isConsideredActive;

    QStyleOptionViewItem adjustedOption = option;

    //! Remove the focus dotted lines
    adjustedOption.state = (adjustedOption.state & ~QStyle::State_HasFocus);
    adjustedOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;

    painter->setRenderHint(QPainter::Antialiasing, true);

    int indicatorLength = INDICATORCHANGESLENGTH + INDICATORCHANGESMARGIN * 2;
    QRect optionRect = (qApp->layoutDirection() == Qt::RightToLeft) ? QRect(option.rect.x() + indicatorLength, option.rect.y(), option.rect.width() - indicatorLength, option.rect.height()) :
                                                                      QRect(option.rect.x(), option.rect.y(), option.rect.width() - indicatorLength, option.rect.height());

    adjustedOption.rect = optionRect;

    if (isLocked || isConsideredActive) {
        QStandardItemModel *model = (QStandardItemModel *) index.model();

        bool active = Latte::isActive(option);
        bool enabled = Latte::isEnabled(option);
        bool selected = Latte::isSelected(option);
        bool focused = Latte::isFocused(option);
        bool hovered = Latte::isHovered(option);

        //! font metrics
        QFontMetrics fm(option.font);
        int textWidth = fm.boundingRect(name).width();
        int thick = optionRect.height();
        int length = drawTwoIcons ? (2*thick /*+ 2*/) : thick;

        int startWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? length : 0;
        int endWidth = (qApp->layoutDirection() == Qt::RightToLeft) ? 0 : length;

        QRect destinationS(optionRect.x(), optionRect.y(), startWidth, thick);
        QRect destinationE(optionRect.x() + optionRect.width() - endWidth, optionRect.y(), endWidth, thick);

        QStyleOptionViewItem myOptionS = adjustedOption;
        QStyleOptionViewItem myOptionE = adjustedOption;
        QStyleOptionViewItem myOptionMain = adjustedOption;

        myOptionMain.font.setBold(isActive);
        myOptionMain.font.setItalic(isChanged);

        myOptionS.rect = destinationS;
        myOptionE.rect = destinationE;
        myOptionMain.rect.moveLeft(optionRect.x() + startWidth);
        myOptionMain.rect.setWidth(optionRect.width() - startWidth - endWidth);

        QStyledItemDelegate::paint(painter, myOptionMain, index);

        //! draw background below icons
        //! HIDDENTEXTCOLUMN is just needed to draw empty background rectangles
        QStyledItemDelegate::paint(painter, myOptionS, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));
        QStyledItemDelegate::paint(painter, myOptionE, model->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

        //! First Icon
        QIcon firstIcon = isLocked && !drawTwoIcons ? QIcon::fromTheme("object-locked") : QIcon::fromTheme("favorites");
        QIcon::Mode mode = ((active && (selected || focused)) ? QIcon::Selected : QIcon::Normal);

        if (qApp->layoutDirection() == Qt::LeftToRight) {
            int firstIconX = optionRect.x() + optionRect.width() - endWidth;
            painter->drawPixmap(QRect(firstIconX, optionRect.y(), thick, thick), firstIcon.pixmap(thick, thick, mode));

            //debug
            //painter->drawLine(firstIconX, optionRect.y(), firstIconX, optionRect.y()+thick);
            //painter->drawLine(firstIconX+thick - 1, optionRect.y(), firstIconX+thick - 1, optionRect.y()+thick);

            if (drawTwoIcons) {
                int secondIconX = optionRect.x() + optionRect.width() - thick;
                QIcon secondIcon = QIcon::fromTheme("object-locked");
                painter->drawPixmap(QRect(secondIconX, optionRect.y(), thick, thick), secondIcon.pixmap(thick, thick, mode));

                //debug
                //painter->drawLine(secondIconX, optionRect.y(), secondIconX, optionRect.y()+thick);
                //painter->drawLine(secondIconX + thick - 1, optionRect.y(), secondIconX + thick - 1,optionRect.y()+thick);
            }
        } else {
            painter->drawPixmap(QRect(optionRect.x(), optionRect.y(), thick, thick), firstIcon.pixmap(thick, thick, mode));

            if (drawTwoIcons) {
                QIcon secondIcon = QIcon::fromTheme("object-locked");
                painter->drawPixmap(QRect(optionRect.x() + thick, optionRect.y(), thick, thick), secondIcon.pixmap(thick, thick, mode));
            }
        }

        //! in order to paint also the background
        QStyleOptionViewItem indicatorOption = adjustedOption;
        indicatorOption.rect = option.rect;
        drawHasChangesIndicator(painter, indicatorOption, index);
        return;
    }

    //! in order to paint also the background
    QStyleOptionViewItem indicatorOption = adjustedOption;
    indicatorOption.rect = option.rect;
    drawHasChangesIndicator(painter, indicatorOption, index);

    //! Draw valid text area
    adjustedOption.font.setBold(isActive);
    adjustedOption.font.setItalic(isChanged);

    QStyledItemDelegate::paint(painter, adjustedOption, index);
}

}
}
}
}
