#include "colorcmbboxdelegate.h"

#include <QComboBox>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QPainter>
#include <QString>

#include <iostream>

ColorCmbBoxDelegate::ColorCmbBoxDelegate(QObject *parent, QString iconsPath)
    : QItemDelegate(parent),
      m_iconsPath(iconsPath)
{
    Items.push_back("Test0");
    Items.push_back("Test1");
    Items.push_back("Test2");
    Items.push_back("Test3");
    Items.push_back("Test4");
    Items.push_back("Test5");
    Items.push_back("Test6");
    Items.push_back("Test7");
    Items.push_back("Test8");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
    Items.push_back("Test...");
}

QWidget *ColorCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);

    //QString colorPath = "/usr/share/plasma/plasmoids/org.kde.latte.containment/contents/icons/blueprint.jpg";

    for (unsigned int i = 0; i < Items.size(); ++i) {
        editor->addItem(Items[i].c_str());
    }

    return editor;
}

void ColorCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    comboBox->setCurrentIndex(value);
}

void ColorCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
}

void ColorCmbBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ColorCmbBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    //QString text = Items[index.row()].c_str();
    //myOption.text = text;
    QVariant value = index.data(Qt::BackgroundRole);

    if (value.isValid()) {
        QString colorPath = m_iconsPath + value.toString() + "print.jpg";
        QBrush colorBrush;
        colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

        painter->setBrush(colorBrush);
        painter->drawRect(QRect(option.rect.x(), option.rect.y(),
                                option.rect.width(), option.rect.height()));
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

