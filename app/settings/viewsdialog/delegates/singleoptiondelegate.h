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

#ifndef SINGLEOPTIONDELEGATE_H
#define SINGLEOPTIONDELEGATE_H

// local
#include "../../../data/genericdata.h"

// Qt
#include <QMenu>
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

class SingleOption : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SingleOption(QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
//    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
//    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

};

}
}
}
}

#endif
