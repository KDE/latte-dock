#include "activitycmbboxdelegate.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QPainter>
#include <QString>

#include <KActivities/Info>

ActivityCmbBoxDelegate::ActivityCmbBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    auto *configDialog = qobject_cast<Latte::LatteConfigDialog *>(parent);

    if (configDialog) {
        m_configDialog = configDialog;
    }
}

QWidget *ActivityCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList availableActivities = m_configDialog->availableActivities();
    QStringList activities = m_configDialog->activities();

    QStringList shownActivities;

    foreach (auto activity, activities) {
        if (assignedActivities.contains(activity) || availableActivities.contains(activity)) {
            shownActivities.append(activity);
        }
    }

    for (unsigned int i = 0; i < shownActivities.count(); ++i) {

        KActivities::Info info(shownActivities[i]);

        QString indicator = "    ";

        if (assignedActivities.contains(shownActivities[i])) {
            indicator = QString::fromUtf8("\u2714") + " ";
        }

        if (info.state() != KActivities::Info::Invalid) {
            editor->addItem(QIcon::fromTheme(info.icon()), QString(indicator + info.name()), QVariant(shownActivities[i]));
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

    int pos = -1;

    if (assignedActivities.count() > 0) {
        pos = comboBox->findData(QVariant(assignedActivities[0]));
    }

    comboBox->setCurrentIndex(pos);
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
        myOption.text = assignedActivitiesText(index);
    }

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOption, painter);
}

QString ActivityCmbBoxDelegate::assignedActivitiesText(const QModelIndex &index) const
{
    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

    QString finalText;

    if (assignedActivities.count() > 0) {
        for (int i = 0; i < assignedActivities.count(); ++i) {
            KActivities::Info info(assignedActivities[i]);

            if (info.state() != KActivities::Info::Invalid) {
                if (i > 0) {
                    finalText += ", ";
                }

                finalText += info.name();
            }
        }
    }

    return finalText;
}
