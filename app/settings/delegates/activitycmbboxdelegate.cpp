/*
*  Copyright 2017-2018 Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "activitycmbboxdelegate.h"

#include "../settingsdialog.h"

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QPainter>
#include <QString>
#include <QTextDocument>

#include <KActivities/Info>

ActivityCmbBoxDelegate::ActivityCmbBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    auto *settingsDialog = qobject_cast<Latte::SettingsDialog *>(parent);

    if (settingsDialog) {
        m_settingsDialog = settingsDialog;
    }
}

QWidget *ActivityCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor =  new QComboBox(parent);

    //! use focusPolicy as flag in order to update activities only when the user is clicking in the popup
    //! it was the only way I found to communicate between the activated (const) signal and the
    //! setEditorData (const) function
    editor->setFocusPolicy(Qt::StrongFocus);

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList availableActivities = m_settingsDialog->availableActivities();
    QStringList activities = m_settingsDialog->activities();

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
        editor->setFocusPolicy(Qt::ClickFocus);
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

    if (editor->focusPolicy() != Qt::ClickFocus) {
        return;
    }

    editor->setFocusPolicy(Qt::StrongFocus);

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
    QStyleOptionViewItem myOptions = option;
    painter->save();

    QStringList assignedActivities = index.model()->data(index, Qt::UserRole).toStringList();

    if (assignedActivities.count() > 0) {
        myOptions.text = assignedActivitiesText(index);

        QTextDocument doc;
        QString css;
        QString activitiesText = myOptions.text;

        QBrush nBrush;

        if ((option.state & QStyle::State_Active) && (option.state & QStyle::State_Selected)) {
            nBrush = option.palette.brush(QPalette::Active, QPalette::HighlightedText);
        } else {
            nBrush = option.palette.brush(QPalette::Inactive, QPalette::Text);
        }

        css = QString("body { color : %1; }").arg(nBrush.color().name());

        doc.setDefaultStyleSheet(css);
        doc.setHtml("<body>" + myOptions.text + "</body>");

        myOptions.text = "";
        myOptions.widget->style()->drawControl(QStyle::CE_ItemViewItem, &myOptions, painter);

        //we need an offset to be in the same vertical center of TextEdit
        int offsetY = ((myOptions.rect.height() - doc.size().height()) / 2);

        if ((qApp->layoutDirection() == Qt::RightToLeft) && !activitiesText.isEmpty()) {
            int textWidth = doc.size().width();

            painter->translate(qMax(myOptions.rect.left(), myOptions.rect.right() - textWidth), myOptions.rect.top() + offsetY + 1);
        } else {
            painter->translate(myOptions.rect.left(), myOptions.rect.top() + offsetY + 1);
        }

        QRect clip(0, 0, myOptions.rect.width(), myOptions.rect.height());
        doc.drawContents(painter, clip);
    } else {
        QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &myOptions, painter);
    }

    painter->restore();
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

                bool isActive{false};

                if ((info.state() == KActivities::Info::Running) || (info.state() == KActivities::Info::Starting)) {
                    isActive = true;
                }

                finalText += isActive ? "<b>" + info.name() + "</b>" : info.name();
            }
        }
    }

    return finalText;
}
