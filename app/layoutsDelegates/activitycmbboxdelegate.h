#ifndef ACTIVITYCMBBOXDELEGATE_H
#define ACTIVITYCMBBOXDELEGATE_H

#include "../latteconfigdialog.h"

#include <QItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

namespace Latte {
class LayoutManager;
}

class ActivityCmbBoxDelegate : public QItemDelegate {
    Q_OBJECT
public:
    ActivityCmbBoxDelegate(QObject *parent);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QString assignedActivitiesText(const QModelIndex &index) const;

    Latte::LatteConfigDialog *m_configDialog{nullptr};
};

#endif
