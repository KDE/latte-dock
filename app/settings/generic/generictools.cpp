/*
 * Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "generictools.h"

// Qt
#include <QApplication>
#include <QDebug>
#include <QStyle>
#include <QTextDocument>

namespace Latte {

const int ICONMARGIN = 1;
const int INDICATORCHANGESLENGTH = 6;
const int INDICATORCHANGESMARGIN = 5;
const int MARGIN = 2;

bool isEnabled(const QStyleOption &option)
{
    if (option.state & QStyle::State_Enabled) {
        return true;
    }

    return false;
}

bool isActive(const QStyleOption &option)
{
    if (option.state & QStyle::State_Active) {
        return true;
    }

    return false;
}

bool isSelected(const QStyleOption &option)
{
    if (option.state & QStyle::State_Selected) {
        return true;
    }

    return false;
}

bool isHovered(const QStyleOption &option)
{
    if (option.state & QStyle::State_MouseOver) {
        return true;
    }

    return false;
}

bool isFocused(const QStyleOption &option)
{
    if (option.state & QStyle::State_HasFocus) {
        return true;
    }

    return false;
}

bool isTextCentered(const QStyleOptionViewItem &option)
{
    if (option.displayAlignment & Qt::AlignHCenter) {
        return true;
    }

    return false;
}

QPalette::ColorGroup colorGroup(const QStyleOption &option)
{
    if (!isEnabled(option)) {
        return QPalette::Disabled;
    }

    if (isActive(option) || isFocused(option)) {
        return QPalette::Active;
    }

    if (!isActive(option) && isSelected(option)) {
        return QPalette::Inactive;
    }

    return QPalette::Normal;
}

QStringList subtracted(const QStringList &original, const QStringList &current)
{
    QStringList subtract;

    for(int i=0; i<original.count(); ++i) {
        if (!current.contains(original[i])) {
            subtract << original[i];
        }
    }

    return subtract;
}

void drawFormattedText(QPainter *painter, const QStyleOptionViewItem &option, const float textOpacity)
{
    painter->save();

    bool isTextCentered = Latte::isTextCentered(option);

    QPalette::ColorRole applyColor = Latte::isSelected(option) ? QPalette::HighlightedText : QPalette::Text;
    QBrush nBrush = option.palette.brush(Latte::colorGroup(option), applyColor);

    QColor brushColor = nBrush.color();
    brushColor.setAlphaF(textOpacity);

    QString css = QString("body { color : %1;}").arg(brushColor.name(QColor::HexArgb));

    QTextDocument doc;
    doc.setDefaultStyleSheet(css);
    doc.setHtml("<body>" + option.text + "</body>");

    QStyleOptionViewItem tempOptions = option;
    tempOptions.text = "";
    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &tempOptions, painter);

    //we need an offset to be in the same vertical center of TextEdit
    int offsetY = ((option.rect.height() - doc.size().height()) / 2);
    int textWidth = doc.size().width();
    int textY = option.rect.top() + offsetY + 1;

    if (isTextCentered) {
        int textX = qMax(0, (option.rect.width() / 2) - (textWidth/2));
        painter->translate(option.rect.left() + textX, textY);
    } else if (qApp->layoutDirection() == Qt::RightToLeft) {
        painter->translate(qMax(option.rect.left(), option.rect.right() - textWidth), textY);
    } else {
        painter->translate(option.rect.left(), textY);
    }

    QRect clip(0, 0, option.rect.width(), option.rect.height());
    doc.drawContents(painter, clip);

    painter->restore();
}

void drawLayoutIcon(QPainter *painter, const QStyleOption &option, const QRect &target, const Latte::Data::LayoutIcon &icon)
{   
    bool active = Latte::isActive(option);
    bool selected = Latte::isSelected(option);
    bool focused = Latte::isFocused(option);

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (icon.isBackgroundFile) {
        int backImageMargin = qMin(option.rect.height()/4, ICONMARGIN+2);
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

QRect remainedFromIcon(const QStyleOption &option, Qt::AlignmentFlag alignment)
{
    int iconsize = option.rect.height() - 2*MARGIN;
    int total = iconsize + 2*ICONMARGIN + 2*MARGIN;

    Qt::AlignmentFlag curalign = alignment;

    if (qApp->layoutDirection() == Qt::LeftToRight) {
        curalign = alignment;
    } else {
        curalign = alignment == Qt::AlignLeft ? Qt::AlignRight : Qt::AlignLeft;
    }


    QRect optionRemainedRect = (curalign == Qt::AlignLeft) ? QRect(option.rect.x() + total, option.rect.y(), option.rect.width() - total, option.rect.height()) :
                                                             QRect(option.rect.x(), option.rect.y(), option.rect.width() - total, option.rect.height());

    return optionRemainedRect;
}

void drawIconBackground(QPainter *painter, const QStyle *style, const QStyleOptionMenuItem &option, Qt::AlignmentFlag alignment)
{
    int iconsize = option.rect.height() - 2*MARGIN;
    int total = iconsize + 2*ICONMARGIN + 2*MARGIN;

    QStyleOptionMenuItem iconOption = option;
    iconOption.text = "";
    //! Remove the focus dotted lines
    //   iconOption.state = (option.state & ~QStyle::State_HasFocus);

    Qt::AlignmentFlag curalign = alignment;

    if (qApp->layoutDirection() == Qt::LeftToRight) {
        curalign = alignment;
    } else {
        curalign = alignment == Qt::AlignLeft ? Qt::AlignRight : Qt::AlignLeft;
    }

    if (curalign == Qt::AlignLeft) {
        iconOption.rect = QRect(option.rect.x(), option.rect.y(), total, option.rect.height());
    } else {
        iconOption.rect = QRect(option.rect.x() + option.rect.width() - total, option.rect.y(), total, option.rect.height());
    }

    style->drawControl(QStyle::CE_MenuItem, &iconOption, painter);
}

void drawIconBackground(QPainter *painter, const QStyleOptionViewItem &option, Qt::AlignmentFlag alignment)
{
    int iconsize = option.rect.height() - 2*MARGIN;
    int total = iconsize + 2*ICONMARGIN + 2*MARGIN;

    QStyleOptionViewItem iconOption = option;
    iconOption.text = "";

    //! Remove the focus dotted lines
    iconOption.state = (option.state & ~QStyle::State_HasFocus);

    Qt::AlignmentFlag curalign = alignment;

    if (qApp->layoutDirection() == Qt::LeftToRight) {
        curalign = alignment;
    } else {
        curalign = alignment == Qt::AlignLeft ? Qt::AlignRight : Qt::AlignLeft;
    }

    if (curalign == Qt::AlignLeft) {
        iconOption.rect = QRect(option.rect.x(), option.rect.y(), total, option.rect.height());
    } else {
        iconOption.rect = QRect(option.rect.x() + option.rect.width() - total, option.rect.y(), total, option.rect.height());
    }

    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &iconOption, painter);
}

void drawIcon(QPainter *painter, const QStyleOption &option, const QString &icon, Qt::AlignmentFlag alignment)
{
    int iconsize = option.rect.height() - 2*MARGIN;
    int total = iconsize + 2*ICONMARGIN + 2*MARGIN;

    bool active = Latte::isActive(option);
    bool selected = Latte::isSelected(option);
    bool focused = Latte::isFocused(option);

    QIcon::Mode mode = ((active && (selected || focused)) ? QIcon::Selected : QIcon::Normal);

    Qt::AlignmentFlag curalign = alignment;

    if (qApp->layoutDirection() == Qt::LeftToRight) {
        curalign = alignment;
    } else {
        curalign = alignment == Qt::AlignLeft ? Qt::AlignRight : Qt::AlignLeft;
    }

    QRect target;

    if (curalign == Qt::AlignLeft) {
        target = QRect(option.rect.x() + MARGIN + ICONMARGIN, option.rect.y(), iconsize, iconsize);
    } else {
        target = QRect(option.rect.x() + option.rect.width() - total + ICONMARGIN + MARGIN, option.rect.y(), iconsize, iconsize);
    }

    painter->drawPixmap(target, QIcon::fromTheme(icon).pixmap(target.height(), target.height(), mode));
}

QRect remainedFromChangesIndicator(const QStyleOptionViewItem &option)
{
    int tsize{INDICATORCHANGESLENGTH + INDICATORCHANGESMARGIN*2};

    QRect optionRemainedRect = (qApp->layoutDirection() == Qt::RightToLeft) ? QRect(option.rect.x() + tsize, option.rect.y(), option.rect.width() - tsize, option.rect.height()) :
                                                                              QRect(option.rect.x(), option.rect.y(), option.rect.width() - tsize, option.rect.height());

    return optionRemainedRect;
}

void drawChangesIndicatorBackground(QPainter *painter, const QStyleOptionViewItem &option)
{
    int tsize{INDICATORCHANGESLENGTH + INDICATORCHANGESMARGIN*2};

    QStyleOptionViewItem indicatorOption = option;
    indicatorOption.text = "";
    //! Remove the focus dotted lines
    indicatorOption.state = (option.state & ~QStyle::State_HasFocus);

    if (qApp->layoutDirection() == Qt::RightToLeft) {
        indicatorOption.rect = QRect(option.rect.x(), option.rect.y(), tsize, option.rect.height());
    } else {
        indicatorOption.rect = QRect(option.rect.x() + option.rect.width() - tsize, option.rect.y(), tsize, option.rect.height());
    }

    option.widget->style()->drawControl(QStyle::CE_ItemViewItem, &indicatorOption, painter);
}

void drawChangesIndicator(QPainter *painter, const QStyleOptionViewItem &option)
{
    //! draw changes circle indicator
    int csize{INDICATORCHANGESLENGTH};
    int tsize{INDICATORCHANGESLENGTH + INDICATORCHANGESMARGIN*2};

    painter->save();

    QRect changesRect = (qApp->layoutDirection() == Qt::RightToLeft) ? QRect(option.rect.x() + INDICATORCHANGESMARGIN, option.rect.y() + option.rect.height()/2 - csize/2, csize, csize) :
                                                                       QRect(option.rect.x() + option.rect.width() - csize - INDICATORCHANGESMARGIN, option.rect.y() + option.rect.height()/2 - csize/2, csize, csize);

    QColor plasmaOrange(246, 116, 0); //orangish color used from plasma systemsettings #f67400
    QBrush backBrush(plasmaOrange);
    QPen pen; pen.setWidth(1);
    pen.setColor(plasmaOrange);

    painter->setBrush(backBrush);
    painter->setPen(pen);
    painter->drawEllipse(changesRect);

    painter->restore();
}

int screenMaxLength(const QStyleOption &option)
{
    int scr_maxlength = option.rect.height() * 1.7;

    //! provide odd screen_maxlength
    if (scr_maxlength % 2 == 0) {
        scr_maxlength--;
    }

    return scr_maxlength;
}

QRect remainedFromScreenDrawing(const QStyleOption &option)
{
    int total_length = screenMaxLength(option) + MARGIN * 2 + 1;

    QRect optionRemainedRect = (qApp->layoutDirection() == Qt::RightToLeft) ? QRect(option.rect.x(), option.rect.y(), option.rect.width() - total_length, option.rect.height()) :
                                                                              QRect(option.rect.x() + total_length, option.rect.y(), option.rect.width() - total_length, option.rect.height());

    return optionRemainedRect;
}

void drawScreenBackground(QPainter *painter, const QStyle *style, const QStyleOptionViewItem &option)
{
    int total_length = screenMaxLength(option) + MARGIN * 2 + 1;

    QStyleOptionViewItem screenOption = option;
    screenOption.text = "";
    //! Remove the focus dotted lines
    screenOption.state = (option.state & ~QStyle::State_HasFocus);

    if (qApp->layoutDirection() == Qt::RightToLeft) {
        screenOption.rect = QRect(option.rect.x() + option.rect.width() - total_length, option.rect.y(), total_length, option.rect.height());
    } else {
        screenOption.rect = QRect(option.rect.x(), option.rect.y(), total_length, option.rect.height());
    }

    style->drawControl(QStyle::CE_ItemViewItem, &screenOption, painter);
}

void drawScreenBackground(QPainter *painter, const QStyle *style, const QStyleOptionMenuItem &option)
{
    int total_length = screenMaxLength(option) + MARGIN * 2 + 1;

    QStyleOptionMenuItem screenOption = option;
    screenOption.text = "";
    //! Remove the focus dotted lines
    screenOption.state = (option.state & ~QStyle::State_HasFocus);

    if (qApp->layoutDirection() == Qt::RightToLeft) {
        screenOption.rect = QRect(option.rect.x() + option.rect.width() - total_length, option.rect.y(), total_length, option.rect.height());
    } else {
        screenOption.rect = QRect(option.rect.x(), option.rect.y(), total_length, option.rect.height());
    }

    style->drawControl(QStyle::CE_MenuItem, &screenOption, painter);
}

QRect drawScreen(QPainter *painter, const QStyleOption &option, QRect screenGeometry, const float brushOpacity)
{
    float scr_ratio = (float)screenGeometry.width() / (float)screenGeometry.height();
    bool isVertical = (scr_ratio < 1.0);

    int scr_maxlength = screenMaxLength(option);
    int scr_maxthickness = option.rect.height() - MARGIN * 2;

    int total_length = scr_maxlength + MARGIN * 2;
    int pen_width = 2;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    float scr_maxratio = ((float)scr_maxlength) / (float)(scr_maxthickness - 2*MARGIN);
    scr_ratio = qMin(qMax((float)0.75, scr_ratio), (float)scr_maxratio);
    int scr_height = (!isVertical ? option.rect.height() - MARGIN * 6 : option.rect.height() - MARGIN * 4);
    int scr_width = scr_ratio * scr_height;

    //! provide even screen width and height
    if (scr_width % 2 == 1) {
        scr_width++;
    }

    //! provide even screen width and height
    if (scr_height % 2 == 0) {
        scr_height++;
    }

    QRect screenMaximumRect = (qApp->layoutDirection() == Qt::RightToLeft) ?
                QRect(option.rect.x() + option.rect.width() - scr_maxlength - MARGIN, option.rect.y() + MARGIN, scr_maxlength, scr_maxthickness - 1) :
                QRect(option.rect.x() + MARGIN , option.rect.y() + MARGIN, scr_maxlength, scr_maxthickness - 1);

    int topScreenMargin = (screenMaximumRect.height() - scr_height) / 2;
    int leftScreenMargin = (screenMaximumRect.width() - scr_width) / 2;

    QRect screenRect(screenMaximumRect.x() + leftScreenMargin + MARGIN/2, screenMaximumRect.y() + topScreenMargin, scr_width, scr_height);

    QRect screenAvailableRect(screenRect.x() + pen_width - 1, screenRect.y() + pen_width - 1, screenRect.width() - pen_width - 1, screenRect.height() - pen_width - 1);

    bool selected = Latte::isSelected(option);
    QPalette::ColorRole textColorRole = selected ? QPalette::HighlightedText : QPalette::Text;

    QPen pen; pen.setWidth(pen_width);
    QColor pencolor = option.palette.color(Latte::colorGroup(option), textColorRole);
    pencolor.setAlphaF(brushOpacity);
    pen.setColor(pencolor);

    painter->setPen(pen);
    painter->drawRect(screenRect);

    pen.setWidth(1);
    painter->setPen(pen);

    int basex = screenRect.x() + (screenRect.width()/2) - 4;
    int basey = screenRect.y() + screenRect.height() + 2;

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawLine(basex , basey, basex + 8, basey);

    // debug screen maximum available rect
    //painter->drawRect(screenMaximumRect);

    painter->restore();

    return screenAvailableRect;
}

void drawView(QPainter *painter, const QStyleOption &option, const Latte::Data::View &view, const QRect &availableScreenRect, const float brushOpacity)
{
    int thick = 4;
    painter->save();

    bool selected = Latte::isSelected(option);
    QPalette::ColorRole viewColorRole = !selected ? QPalette::Highlight : QPalette::Text;
    QPen pen; pen.setWidth(thick);
    QColor pencolor = option.palette.color(Latte::colorGroup(option), viewColorRole);
    pencolor.setAlphaF(brushOpacity);
    pen.setColor(pencolor);
    painter->setPen(pen);

    int x = availableScreenRect.x();
    int y = availableScreenRect.y();

    int length = view.isVertical() ? availableScreenRect.height() - thick + 1 : availableScreenRect.width() - thick + 1;
    int max_length = length;

    if (view.alignment != Latte::Types::Justify) {
        length = 0.5 * length;
    }

    //! provide even screen length
    if (length % 2 == 1) {
        length = qMin(max_length, length + 1);
    }

    int screen_edge = (view.screenEdgeMargin > 0) ? 2 : 0;

    if (view.edge == Plasma::Types::TopEdge) {
        y = availableScreenRect.y() + thick/2 + screen_edge;
    } else if (view.edge == Plasma::Types::BottomEdge) {
        y = availableScreenRect.y() + availableScreenRect.height() - 1 - screen_edge;
    } else if (view.edge == Plasma::Types::LeftEdge) {
        x = availableScreenRect.x() + thick/2 + screen_edge;
    } else if (view.edge == Plasma::Types::RightEdge) {
        x = availableScreenRect.x() + availableScreenRect.width() - 1 - screen_edge;
    }

    if (view.isHorizontal()) {
        if (view.alignment == Latte::Types::Left) {
            x = availableScreenRect.x() + thick / 2;
        } else if (view.alignment == Latte::Types::Right) {
            x = availableScreenRect.x() + availableScreenRect.width() - length - 1;
        } else if (view.alignment == Latte::Types::Center) {
            int leftmargin = (availableScreenRect.width()/2) - (length/2);
            x = availableScreenRect.x() + leftmargin + 1;
        } else if (view.alignment == Latte::Types::Justify) {
            x = availableScreenRect.x() + thick / 2;
        }
    } else if (view.isVertical()) {
        if (view.alignment == Latte::Types::Top) {
            y = availableScreenRect.y() + thick / 2;
        } else if (view.alignment == Latte::Types::Bottom) {
            y = availableScreenRect.y() + availableScreenRect.height() - length - 1;
        } else if (view.alignment == Latte::Types::Center) {
            int topmargin = (availableScreenRect.height()/2) - (length/2);
            y = availableScreenRect.y() + topmargin + 1;
        } else if (view.alignment == Latte::Types::Justify) {
            y = availableScreenRect.y() + thick / 2;
        }
    }

    if (view.isHorizontal()) {
        painter->drawLine(x, y, x + length, y);
    } else if (view.isVertical()) {
        painter->drawLine(x, y, x, y + length);
    }

    painter->restore();
}




}

