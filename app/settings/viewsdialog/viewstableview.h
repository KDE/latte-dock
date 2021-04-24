/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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
