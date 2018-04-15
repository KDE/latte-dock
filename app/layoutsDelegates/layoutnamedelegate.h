#include <QStyledItemDelegate>

class LayoutNameDelegate : public QStyledItemDelegate {
public:
    LayoutNameDelegate(QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};
