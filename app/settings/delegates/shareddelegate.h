/*
*  Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef SHAREDDELEGATE_H
#define SHAREDDELEGATE_H

// local
#include "../data/layoutstable.h"

// Qt
#include <QItemDelegate>

class QModelIndex;
class QWidget;

namespace Latte {
namespace Settings {
namespace Layouts {
namespace Delegates {

class Shared : public QItemDelegate
{
    Q_OBJECT
public:
    Shared(QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void updateButtonText(QWidget *editor, const Data::LayoutsTable &allLayouts) const;

    QString joined(const Data::LayoutsTable &layouts, bool formatText = true) const;
};

}
}
}
}

#endif
