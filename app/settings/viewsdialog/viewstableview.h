/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWSTABLEVIEW_H
#define VIEWSTABLEVIEW_H

// Qt
#include <QTableView>
#include <QMouseEvent>

namespace Latte {
namespace Settings {
namespace View {

class ViewsTableView : public QTableView
{
    Q_OBJECT
public:
    ViewsTableView(QWidget *parent = nullptr);

signals:
    void selectionsChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;


};

}
}
}
#endif
