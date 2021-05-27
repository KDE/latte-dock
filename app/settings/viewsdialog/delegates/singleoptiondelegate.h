/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SINGLEOPTIONDELEGATE_H
#define SINGLEOPTIONDELEGATE_H

// local
#include "singletextdelegate.h"
#include "../../../data/genericdata.h"

// Qt
#include <QMenu>
#include <QPainter>
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

class SingleOption : public SingleText
{
    Q_OBJECT
public:
    SingleOption(QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void updateButton(QWidget *editor, const QString &text) const;

};

}
}
}
}

#endif
