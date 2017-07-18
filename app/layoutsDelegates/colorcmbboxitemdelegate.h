#ifndef COLORCMBBOXITEMDELEGATE_H
#define COLORCMBBOXITEMDELEGATE_H

#include <QAbstractItemDelegate>

class ColorCmbBoxItemDelegate : public QAbstractItemDelegate {
    Q_OBJECT
public:
    ColorCmbBoxItemDelegate(QObject *parent = 0, QString iconsPath = QString());

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QString m_iconsPath;

};

#endif
