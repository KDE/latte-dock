#ifndef COLORCMBBOXITEMDELEGATE_H
#define COLORCMBBOXITEMDELEGATE_H

#include <QAbstractItemDelegate>

class ColorCmbBoxItemDelegate : public QAbstractItemDelegate {
    Q_OBJECT
public:
    ColorCmbBoxItemDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif
