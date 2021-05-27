/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "colorcmbitemdelegate.h"

// local
#include "../colorsmodel.h"
#include "../../generic/generictools.h"

// Qt
#include <QDebug>
#include <QImage>
#include <QMargins>
#include <QPainter>
#include <QPainterPath>
#include <QString>

namespace Latte {
namespace Settings {
namespace Details {
namespace Delegate {

ColorCmbBoxItem::ColorCmbBoxItem(QObject *parent)
    : QAbstractItemDelegate(parent)
{
    m_pattern = new Widget::PatternWidget();
}

ColorCmbBoxItem::~ColorCmbBoxItem()
{
    m_pattern->deleteLater();
}

QSize ColorCmbBoxItem::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.rect.width(), 60);
}

void ColorCmbBoxItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    QString name = index.data(Model::Colors::NAMEROLE).toString();
    QString colorPath = index.data(Model::Colors::PATHROLE).toString();
    QString textColor = index.data(Model::Colors::TEXTCOLORROLE).toString();

    bool selected = Latte::isSelected(option);
    painter->setRenderHint(QPainter::Antialiasing, true);

    //! draw background
    if (selected) {
        QPalette::ColorRole selectedColorRole = QPalette::Highlight;
        QColor selectedColor = option.palette.brush(Latte::colorGroup(option), selectedColorRole).color();

        QPainterPath back;
        back.addRect(option.rect);
        painter->fillPath(back, selectedColor);
    }

    //! draw pattern control
    myOption.rect -= QMargins(4,4,4,4);

    m_pattern->setText(name);
    m_pattern->setBackground(colorPath);
    m_pattern->setTextColor(textColor);
    m_pattern->resize(myOption.rect.size());

    myOption.rect.moveTo(option.rect.x() + 6, option.rect.y() + 6);
    m_pattern->render(painter, myOption.rect.topLeft(), QRegion(), QWidget::DrawChildren );
}

}
}
}
}

