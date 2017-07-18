#include "activitycmbboxdelegate.h"

#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QApplication>
#include <QPainter>
#include <QString>

#include "QMultiComboBox.h"

#include <KActivities/Info>

ActivityCmbBoxDelegate::ActivityCmbBoxDelegate(QObject *parent, Latte::LayoutManager *manager)
    : QItemDelegate(parent),
      m_manager(manager)
{
    m_activities = m_manager->activities();
}

QWidget *ActivityCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QMultiComboBox *editor = new QMultiComboBox(parent);

    for (unsigned int i = 0; i < m_activities.count(); ++i) {

        KActivities::Info info(m_activities[i]);

        if (info.state() != KActivities::Info::Invalid) {
            editor->addItem(info.name(), QVariant(m_activities[i]));
        }
    }

    return editor;
}

void ActivityCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QMultiComboBox *comboBox = static_cast<QMultiComboBox *>(editor);
    //QString value = index.model()->data(index, Qt::BackgroundRole).toString();
    //comboBox->setCurrentIndex(Colors.indexOf(value));
}

void ActivityCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    //QComboBox *comboBox = static_cast<QComboBox *>(editor);
    //model->setData(index, comboBox->currentText(), Qt::BackgroundRole);
}

void ActivityCmbBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ActivityCmbBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;
    /*  QVariant value = index.data(Qt::BackgroundRole);

      if (value.isValid()) {
          QString colorPath = m_iconsPath + value.toString() + "print.jpg";
          QBrush colorBrush;
          colorBrush.setTextureImage(QImage(colorPath).scaled(QSize(50, 50)));

          painter->setBrush(colorBrush);
          painter->drawRect(QRect(option.rect.x(), option.rect.y(),
                                  option.rect.width(), option.rect.height()));
      }*/

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

