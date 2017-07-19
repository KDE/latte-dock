#include "activitycmbboxdelegate.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QPainter>
#include <QString>

#include <KActivities/Info>

ActivityCmbBoxDelegate::ActivityCmbBoxDelegate(QObject *parent, Latte::LayoutManager *manager)
    : QItemDelegate(parent),
      m_manager(manager)
{
    m_activities = m_manager->activities();
}

QWidget *ActivityCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);

    for (unsigned int i = 0; i < m_activities.count(); ++i) {

        KActivities::Info info(m_activities[i]);

        if (info.state() != KActivities::Info::Invalid) {
            editor->addItem(QIcon::fromTheme(info.icon()), info.name(), QVariant(m_activities[i]));
        }
    }

    return editor;
}

void ActivityCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    //QComboBox *comboBox = static_cast<QComboBox *>(editor);
    //QString value = index.model()->data(index, Qt::BackgroundRole).toString();
    //comboBox->setCurrentIndex(Colors.indexOf(value));
}

void ActivityCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);

    for (int i = 0; i < comboBox->count(); ++i) {
        qDebug() << i << ". " << comboBox->itemData(i);
    }

    bool value = index.model()->data(index, Qt::UserRole).toBool();
    qDebug() << " model ::: " << value;

    //model->setData(index, comboBox->currentText(), Qt::UserDataRole);
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

