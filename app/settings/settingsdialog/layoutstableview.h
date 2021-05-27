/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTSTABLEVIEW_H
#define LAYOUTSTABLEVIEW_H

// Qt
#include <QLabel>
#include <QPaintEvent>
#include <QTableView>
#include <QDragEnterEvent>

namespace Latte {
namespace Settings {
namespace View {

class LayoutsTableView : public QTableView
{
    Q_OBJECT
public:
    LayoutsTableView(QWidget *parent = nullptr);

    void dragEntered(QDragEnterEvent *event);
    void dragLeft();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QLabel *m_overlayDropMessage;


};

}
}
}
#endif
