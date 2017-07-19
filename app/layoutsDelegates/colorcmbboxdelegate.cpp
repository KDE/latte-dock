#include "colorcmbboxdelegate.h"
#include "colorcmbboxitemdelegate.h"

#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QPainter>
#include <QString>

ColorCmbBoxDelegate::ColorCmbBoxDelegate(QObject *parent, QString iconsPath, QStringList colors)
    : QItemDelegate(parent),
      m_iconsPath(iconsPath),
      Colors(colors)
{
}

QWidget *ColorCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->setItemDelegate(new ColorCmbBoxItemDelegate(editor, m_iconsPath));

    for (unsigned int i = 0; i < Colors.count(); ++i) {
        if (Colors[i] != "sepia") {
            QPixmap pixmap(50, 50);
            pixmap.fill(QColor(Colors[i]));
            QIcon icon(pixmap);

            editor->addItem(icon, Colors[i]);
        }
    }

    connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [ = ](int index) {
        editor->clearFocus();
    });

    return editor;
}

void ColorCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    QString value = index.model()->data(index, Qt::BackgroundRole).toString();
    comboBox->setCurrentIndex(Colors.indexOf(value));
}

void ColorCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentText(), Qt::BackgroundRole);
}

void ColorCmbBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ColorCmbBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    QVariant value = index.data(Qt::BackgroundRole);

    if (value.isValid()) {
        QString colorPath = m_iconsPath + value.toString() + "print.jpg";
        QBrush colorBrush;
        colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

        painter->setBrush(colorBrush);
        painter->drawRect(QRect(option.rect.x(), option.rect.y(),
                                option.rect.width(), option.rect.height()));
    }

    //QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

