/*
*  Copyright 2019 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "sharedcmbboxdelegate.h"

// local
#include "../settingsdialog.h"

// Qt
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QWidget>
#include <QModelIndex>
#include <QPainter>
#include <QString>
#include <QTextDocument>

// KDE
#include <KActivities/Info>

SharedCmbBoxDelegate::SharedCmbBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    auto *settingsDialog = qobject_cast<Latte::SettingsDialog *>(parent);

    if (settingsDialog) {
        m_settingsDialog = settingsDialog;
    }
}

QWidget *SharedCmbBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QComboBox *editor =  new QComboBox(parent);

    //! use focusPolicy as flag in order to update activities only when the user is clicking in the popup
    //! it was the only way I found to communicate between the activated (const) signal and the
    //! setEditorData (const) function
    editor->setFocusPolicy(Qt::StrongFocus);

    QStringList assignedShares = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList availableShares = m_settingsDialog->availableSharesFor(index.row());

    for (unsigned int i = 0; i < availableShares.count(); ++i) {
        QString indicator = "    ";

        if (assignedShares.contains(availableShares[i])) {
            indicator = QString::fromUtf8("\u2714") + " ";
        }

        editor->addItem(QString(indicator + availableShares[i]), QVariant(availableShares[i]));
    }

    connect(editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [ = ](int index) {
        editor->setFocusPolicy(Qt::ClickFocus);
        editor->clearFocus();
    });

    return editor;
}

void SharedCmbBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    QStringList assignedLayouts = index.model()->data(index, Qt::UserRole).toStringList();

    int pos = -1;

    if (assignedLayouts.count() > 0) {
        pos = comboBox->findData(QVariant(assignedLayouts[0]));
    }

    comboBox->setCurrentIndex(pos);
}

void SharedCmbBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);

    if (editor->focusPolicy() != Qt::ClickFocus) {
        return;
    }

    editor->setFocusPolicy(Qt::StrongFocus);

    QStringList assignedLayouts = index.model()->data(index, Qt::UserRole).toStringList();
    QString selectedLayout = comboBox->currentData().toString();

    if (assignedLayouts.contains(selectedLayout)) {
        assignedLayouts.removeAll(selectedLayout);
    } else {
        assignedLayouts.append(selectedLayout);
    }

    model->setData(index, assignedLayouts, Qt::UserRole);
}

void SharedCmbBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void SharedCmbBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    painter->save();

    QStringList assignedLayouts = index.model()->data(index, Qt::UserRole).toStringList();

    if (assignedLayouts.count() > 0) {
        myOptions.text = assignedLayoutsText(index);

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

QString SharedCmbBoxDelegate::assignedLayoutsText(const QModelIndex &index) const
{
    QStringList assignedLayouts = index.model()->data(index, Qt::UserRole).toStringList();

    QString finalText;

    if (assignedLayouts.count() > 0) {
        for (int i = 0; i < assignedLayouts.count(); ++i) {
            if (i > 0) {
                finalText += ", ";
            }

            finalText += assignedLayouts[i];
        }
    }

    return finalText;
}
