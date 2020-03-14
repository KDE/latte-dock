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
#include "../data/layoutdata.h"
#include "../models/layoutsmodel.h"
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


namespace Latte {
namespace Settings {
namespace Layout {
namespace Delegate {


Shared::Shared(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *Shared::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool inMultiple = index.data(Model::Layouts::INMULTIPLELAYOUTSROLE).toBool();

    Data::LayoutsTable allLayouts = qvariant_cast<Data::LayoutsTable>(index.data(Model::Layouts::ALLLAYOUTSROLE));
    QStringList assignedShares = index.data(Qt::UserRole).toStringList();

    QPushButton *button = new QPushButton(parent);
    PersistentMenu *menu = new PersistentMenu(button);
    button->setMenu(menu);

    menu->setMinimumWidth(option.rect.width());

    for (unsigned int i = 0; i < allLayouts.rowCount(); ++i) {
        if (inMultiple && allLayouts[i].isShared()) {
            continue;
        }

        QAction *action = new QAction(allLayouts[i].editedName());
        action->setData(allLayouts[i].id);
        action->setCheckable(true);
        action->setChecked(assignedShares.contains(allLayouts[i].id));

        if (allLayouts[i].isActive) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }

        menu->addAction(action);

        connect(action, &QAction::toggled, this, [this, button, action, allLayouts]() {
            updateButtonText(button, allLayouts);
        });
    }

    updateButtonText(button, allLayouts);

    return button;
}

void Shared::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Data::LayoutsTable allLayouts = qvariant_cast<Data::LayoutsTable>(index.data(Model::Layouts::ALLLAYOUTSROLE));
    updateButtonText(editor, allLayouts);
}

void Shared::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
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

void Shared::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void Shared::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Data::LayoutsTable allLayouts = qvariant_cast<Data::LayoutsTable>(index.data(Model::Layouts::ALLLAYOUTSROLE));
    QStringList assignedIds = index.data(Qt::UserRole).toStringList();

    Data::LayoutsTable assignedLayouts;

    for (const auto &id : assignedIds) {
        assignedLayouts << allLayouts[id];
    }

    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    painter->save();

    if (assignedLayouts.rowCount() > 0) {
        myOptions.text = joined(assignedLayouts);

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

void Shared::updateButtonText(QWidget *editor, const Data::LayoutsTable &allLayouts) const
{
    if (!editor) {
        return;
    }

    QPushButton *button = static_cast<QPushButton *>(editor);
    Data::LayoutsTable assignedLayouts;

    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            assignedLayouts << allLayouts[action->data().toString()];
        }
    }

    button->setText(joined(assignedLayouts, false));
}

QString Shared::joined(const Data::LayoutsTable &layouts, bool formatText) const
{
    QString finalText;

    int i = 0;

    for (unsigned int i = 0; i < layouts.rowCount(); ++i) {
        if (i > 0) {
            finalText += ", ";
        }

        bool bold {false};

        if (formatText && layouts[i].isActive) {
            bold = true;
        }

        finalText += bold ? "<b>" + layouts[i].editedName() + "</b>" : layouts[i].editedName();
    }

    return finalText;
}

}
}
}
}
