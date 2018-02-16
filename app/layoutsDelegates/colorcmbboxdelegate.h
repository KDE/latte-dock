#ifndef COLORCMBBOXDELEGATE_H
#define COLORCMBBOXDELEGATE_H

#include <QItemDelegate>

class QModelIndex;
class QWidget;
class QVariant;

class ColorCmbBoxDelegate : public QItemDelegate {
    Q_OBJECT
public:
    ColorCmbBoxDelegate(QObject *parent = 0, QString iconsPath = QString(), QStringList colors = QStringList());

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QObject *m_parent{nullptr};

    QStringList Colors;

    QString m_iconsPath;
};
#endif
