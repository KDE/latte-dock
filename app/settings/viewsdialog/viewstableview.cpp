/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewstableview.h"

//! Qt
#include <QDebug>
#include <QModelIndex>


namespace Latte {
namespace Settings {
namespace View {

ViewsTableView::ViewsTableView(QWidget *parent)
    : QTableView(parent)
{
}

void ViewsTableView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex eventIndex = indexAt(event->pos());

    if (!eventIndex.isValid()) {
        clearSelection();
    }

    QTableView::mousePressEvent(event);
}

void ViewsTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QAbstractItemView::selectionChanged(selected, deselected);
    emit selectionsChanged();
}

}
}
}

