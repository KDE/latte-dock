/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutsheaderview.h"

// local
#include "layoutsmodel.h"

// Qt
#include <QAbstractItemModel>
#include <QPainter>


namespace Latte {
namespace Settings {
namespace Layouts {

HeaderView::HeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);
    setSectionsMovable(true);
    setSortIndicatorShown(true);
}

void HeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{   
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (logicalIndex == Model::Layouts::BACKGROUNDCOLUMN) {
        QString text = model()->headerData(Model::Layouts::BACKGROUNDCOLUMN, Qt::Horizontal, Qt::DisplayRole).toString();
        QIcon icon = model()->headerData(Model::Layouts::BACKGROUNDCOLUMN, Qt::Horizontal, Qt::DecorationRole).value<QIcon>();

        if (text.isEmpty() && !icon.isNull()) {
            //! draw centered icon
            QHeaderView::paintSection(painter, rect, Model::Layouts::HIDDENTEXTCOLUMN);

            int margin = 4;
            int thick = rect.height() - 2*margin;
            int iX = rect.x() + (rect.width()/2) - (thick/2);

            painter->drawPixmap(QRect(iX, rect.y() + margin, thick, thick), icon.pixmap(thick, thick, QIcon::Normal));
            return;
        }
    }

    QHeaderView::paintSection(painter, rect, logicalIndex);
}

}
}
}

