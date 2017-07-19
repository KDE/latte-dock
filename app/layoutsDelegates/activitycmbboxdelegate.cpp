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

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

    for (unsigned int i = 0; i < m_activities.count(); ++i) {

        KActivities::Info info(m_activities[i]);

        QString indicator = "    ";

        if (assignedActivities.contains(m_activities[i])) {
            indicator = QString::fromUtf8("\u2714") + " ";
        }

        if (info.state() != KActivities::Info::Invalid) {
            editor->addItem(QIcon::fromTheme(info.icon()), QString(indicator + info.name()), QVariant(m_activities[i]));
        }
    }

    connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [ = ](int index) {
        editor->clearFocus();
    });

    return editor;
}

void ActivityCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

    if (assignedActivities.count() > 0) {
        comboBox->setCurrentIndex(m_activities.indexOf(assignedActivities[0]));
    } else {
        comboBox->setCurrentIndex(-1);
    }
}

void ActivityCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();
    QString selectedActivity = comboBox->currentData().toString();

    if (assignedActivities.contains(selectedActivity)) {
        assignedActivities.removeAll(selectedActivity);
    } else {
        assignedActivities.append(selectedActivity);
    }

    model->setData(index, assignedActivities, Qt::UserRole);
}

void ActivityCmbBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void ActivityCmbBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOption = option;

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

    if (assignedActivities.count() > 0) {
        QString finalText;

        for (int i = 0; i < assignedActivities.count(); ++i) {
            KActivities::Info info(assignedActivities[i]);

            if (info.state() != KActivities::Info::Invalid) {
                if (i > 0) {
                    finalText += ", ";
                }

                finalText += info.name();
            }
        }

        myOption.text = finalText;
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

