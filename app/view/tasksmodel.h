/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWTASKSMODEL_H
#define VIEWTASKSMODEL_H

#include <QAbstractListModel>

namespace PlasmaQuick {
class AppletQuickItem;
}

namespace Latte {
namespace ViewPart {

class TasksModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    TasksModel(QObject *parent = nullptr);

    int count() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

   void addTask(PlasmaQuick::AppletQuickItem *plasmoid);
   void removeTask(PlasmaQuick::AppletQuickItem *plasmoid);

signals:
   void countChanged();

private slots:
   void moveIntoWaitingTasks(PlasmaQuick::AppletQuickItem *plasmoid);
   void restoreFromWaitingTasks(PlasmaQuick::AppletQuickItem *plasmoid);

private:
    QList<PlasmaQuick::AppletQuickItem *> m_tasks;
    QList<PlasmaQuick::AppletQuickItem *> m_tasksWaiting;
};

}
}

#endif
