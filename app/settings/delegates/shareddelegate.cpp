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

#include "shareddelegate.h"

// local
#include "persistentmenu.h"
#include "../settingsdialog.h"
#include "../tools/settingstools.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QMenu>
#include <QWidget>
#include <QModelIndex>
#include <QPainter>
#include <QPushButton>
#include <QString>
#include <QTextDocument>


SharedDelegate::SharedDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    auto *settingsDialog = qobject_cast<Latte::SettingsDialog *>(parent);

    if (settingsDialog) {
        m_settingsDialog = settingsDialog;
    }
}

QWidget *SharedDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStringList assignedShares = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList availableShares = m_settingsDialog->availableSharesFor(index.row());

    QPushButton *button = new QPushButton(parent);
    PersistentMenu *menu = new PersistentMenu(button);
    button->setMenu(menu);

    menu->setMinimumWidth(option.rect.width());

    for (unsigned int i = 0; i < availableShares.count(); ++i) {
        QAction *action = new QAction(m_settingsDialog->nameForId(availableShares[i]));
        action->setData(availableShares[i]);
        action->setCheckable(true);
        action->setChecked(assignedShares.contains(availableShares[i]));
        menu->addAction(action);

        connect(action, &QAction::toggled, this, [this, button, action]() {
            updateButtonText(button);

            if (action->isChecked()) {
                m_settingsDialog->addShareInCurrent(action->data().toString());
            } else {
                m_settingsDialog->removeShareFromCurrent(action->data().toString());
            }
        });
    }

    updateButtonText(button);

    return button;
}

void SharedDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    updateButtonText(editor);
}

void SharedDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QPushButton *button = static_cast<QPushButton *>(editor);

    QStringList assignedLayouts;
    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            assignedLayouts << action->data().toString();
        }
    }

    model->setData(index, assignedLayouts, Qt::UserRole);
}

void SharedDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void SharedDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    painter->save();

    QStringList assignedLayoutsIds = index.model()->data(index, Qt::UserRole).toStringList();
    QStringList assignedLayouts;

    for (const auto &id : assignedLayoutsIds) {
        assignedLayouts << m_settingsDialog->nameForId(id);
    }

    if (assignedLayouts.count() > 0) {
        myOptions.text = joined(assignedLayouts, true);

        QTextDocument doc;
        QString css;
        QString sharesText = myOptions.text;

        QPalette::ColorRole applyColor = Latte::isSelected(option) ? QPalette::HighlightedText : QPalette::Text;
        QBrush nBrush = option.palette.brush(Latte::colorGroup(option), applyColor);

        css = QString("body { color : %1; }").arg(nBrush.color().name());

        doc.setDefaultStyleSheet(css);
        doc.setHtml("<body>" + myOptions.text + "</body>");

        myOptions.text = "";
        myOptions.widget->style()->drawControl(QStyle::CE_ItemViewItem, &myOptions, painter);

        //we need an offset to be in the same vertical center of TextEdit
        int offsetY = ((myOptions.rect.height() - doc.size().height()) / 2);

        if ((qApp->layoutDirection() == Qt::RightToLeft) && !sharesText.isEmpty()) {
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

void SharedDelegate::updateButtonText(QWidget *editor) const
{
    if (!editor) {
        return;
    }
    QPushButton *button = static_cast<QPushButton *>(editor);
    QStringList assignedLayouts;

    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            assignedLayouts << m_settingsDialog->nameForId(action->data().toString());
        }
    }

    button->setText(joined(assignedLayouts));
}

QString SharedDelegate::joined(const QStringList &layouts, bool boldForActive) const
{
    QString finalText;

    int i = 0;

    for (const auto &layoutName : layouts) {
        if (i > 0) {
            finalText += ", ";
        }
        i++;

        bool isActive{false};

        if (boldForActive && m_settingsDialog->isActive(layoutName)) {
            isActive = true;
        }

        finalText += isActive ? "<b>" + layoutName + "</b>" : layoutName;
    }

    return finalText;
}
