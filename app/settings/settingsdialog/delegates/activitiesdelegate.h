/*
    SPDX-FileCopyrightText: 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIVITIESDELEGATE_H
#define ACTIVITIESDELEGATE_H

// local
#include "../../../data/activitydata.h"

// Qt
#include <QMenu>
#include <QStyledItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {

class Activities : public QStyledItemDelegate
{
    Q_OBJECT
public:
    Activities(QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    void updateCurrentActivityAction(QMenu *menu) const;
    void updateButton(QWidget *editor, const Latte::Data::ActivitiesTable &allActivitiesTable) const;

    QString joinedActivities(const QList<Latte::Data::Activity> &activities, const QStringList &originalIds, bool isActive = false, bool formatText = true) const;
};

}
}
}
}

#endif
