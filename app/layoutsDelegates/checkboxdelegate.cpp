#include "checkboxdelegate.h"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected) {
        QPen nPen;
        QBrush nBrush;

        if (option.state & QStyle::State_Active) {
            nBrush = option.palette.highlight();
        } else if (option.state & QStyle::State_MouseOver) {
            nBrush = option.palette.brush(QPalette::Inactive, QPalette::Highlight);
        } else {
            nBrush = option.palette.brush(QPalette::Inactive, QPalette::Highlight);
        }

        painter->setBrush(nBrush);
        nPen.setColor(nBrush.color());

        painter->setPen(nPen);
        painter->drawRect(option.rect);
    }

    QStyledItemDelegate::paint(painter, option, index);
}

bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

//     // make sure that the item is checkable
//     Qt::ItemFlags flags = model->flags(index);
//
//     if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
//         return false;
//
//     // make sure that we have a check state
    QString value{index.data().toString()};
//
//     if (!value.isValid())
//         return false;

    // make sure that we have the right event type
    if (event->type() == QEvent::MouseButtonRelease) {
        if (!option.rect.contains(static_cast<QMouseEvent *>(event)->pos()))
            return false;
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    const QChar CheckMark{0x2714};
    return model->setData(index, value == CheckMark ? QString("") : CheckMark, Qt::DisplayRole);
}
