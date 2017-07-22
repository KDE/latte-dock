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
    QStyleOptionViewItem viewItemOption(option);

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

    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_CheckBoxLabelSpacing);
    QRect newRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                        QSize(option.decorationSize.width() + textMargin * 2, option.decorationSize.height()),
                                        option.rect);

    //viewItemOption.rect = newRect;
    viewItemOption.rect = newRect;
    viewItemOption.decorationAlignment = Qt::AlignCenter;

    QStyledItemDelegate::paint(painter, viewItemOption, index);
}

bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);

    if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);

    if (!value.isValid())
        return false;

    // make sure that we have the right event type
    if (event->type() == QEvent::MouseButtonRelease) {
        QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                              option.decorationSize,
                                              QRect(option.rect.x(), option.rect.y(),
                                                      option.rect.width(), option.rect.height()));

        if (!checkRect.contains(static_cast<QMouseEvent *>(event)->pos()))
            return false;
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
                            ? Qt::Unchecked : Qt::Checked);
    return model->setData(index, state, Qt::CheckStateRole);
}
