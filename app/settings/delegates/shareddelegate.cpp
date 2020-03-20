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


Shared::Shared(Controller::Layouts *parent)
    : QStyledItemDelegate(parent),
      m_controller(parent)
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

    for (int i = 0; i < allLayouts.rowCount(); ++i) {
        if (inMultiple && allLayouts[i].isShared()) {
            continue;
        }

        QAction *action = new QAction(allLayouts[i].name);
        action->setData(allLayouts[i].id);
        action->setCheckable(true);
        action->setChecked(assignedShares.contains(allLayouts[i].id));

        if (allLayouts[i].isActive) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }

        menu->addAction(action);

        connect(action, &QAction::toggled, this, [this, button, index]() {
            updateButtonText(button, index);
        });
    }

    updateButtonText(button, index);

    m_controller->on_sharedToInEditChanged(index.row(), true);

    return button;
}

void Shared::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Data::LayoutsTable allLayouts = qvariant_cast<Data::LayoutsTable>(index.data(Model::Layouts::ALLLAYOUTSROLE));
    updateButtonText(editor, index);
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

    m_controller->on_sharedToInEditChanged(index.row(), false);
}

void Shared::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

void Shared::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool sharedInEdit = index.data(Model::Layouts::SHAREDTOINEDIT).toBool();
    Data::LayoutsTable allLayouts = qvariant_cast<Data::LayoutsTable>(index.data(Model::Layouts::ALLLAYOUTSROLE));
    QStringList assignedIds = index.data(Qt::UserRole).toStringList();    

    Data::LayoutsTable assignedLayouts;

    for (const auto &id : assignedIds) {
        if (allLayouts.containsId(id)) {
            assignedLayouts << allLayouts[id];
        }
    }

    QStyleOptionViewItem myOptions = option;
    //! Remove the focus dotted lines
    myOptions.state = (myOptions.state & ~QStyle::State_HasFocus);
    painter->save();

    if (assignedLayouts.rowCount() > 0) {
        //! indicator
        if (!sharedInEdit) {
            paintSharedToIndicator(painter, myOptions, index);
        }

        //! Text code
        myOptions.text = joined(assignedLayouts);

        int thick = option.rect.height();

        if (qApp->layoutDirection() == Qt::LeftToRight) {
            myOptions.rect = QRect(myOptions.rect.x() + thick, myOptions.rect.y(), myOptions.rect.width() - thick, myOptions.rect.height());
        } else {
            myOptions.rect = QRect(myOptions.rect.x(), myOptions.rect.y(), myOptions.rect.width() - thick, myOptions.rect.height());
        }

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

void Shared::paintSharedToIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Disabled
    bool isSelected{Latte::isSelected(option)};
    QPalette::ColorRole backColorRole = isSelected ? QPalette::Highlight : QPalette::Base;
    QPalette::ColorRole textColorRole = isSelected ? QPalette::HighlightedText : QPalette::Text;

    int space = option.rect.height() / 2;

    //! draw background below icons
    //! HIDDENTEXTCOLUMN is just needed to draw empty background rectangles properly based on states
    QStyleOptionViewItem backOptions = option;
    if (qApp->layoutDirection() == Qt::LeftToRight) {
        backOptions.rect = QRect(option.rect.x(), option.rect.y(), 2 * space, 2 * space);
    } else {
        backOptions.rect = QRect(option.rect.x() + option.rect.width() - (2*space), option.rect.y(), 2 * space, 2 * space);
    }

    QStyledItemDelegate::paint(painter, backOptions, index.model()->index(index.row(), Model::Layouts::HIDDENTEXTCOLUMN));

    // text
    QPen pen(Qt::DotLine);
    QColor textColor = option.palette.brush(Latte::colorGroup(option), textColorRole).color();

    pen.setWidth(2); pen.setColor(textColor);
    int y = option.rect.y()+option.rect.height()/2;

    painter->setPen(pen);

    if (qApp->layoutDirection() == Qt::LeftToRight) {
        int xStart = option.rect.x();
        painter->drawLine(xStart, y, xStart + space, y);

        int xm = option.rect.x() + space;
        int thick = option.rect.height() / 2;
        int ym = option.rect.y() + ((option.rect.height() - thick) / 2);

        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->setBrush(textColor);

        //! draw ending cirlce
        painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
    } else {
        int xEnd = option.rect.x() + option.rect.width();
        painter->drawLine(xEnd, y, xEnd-space, y);

        int xm = option.rect.x() + option.rect.width() - space;
        int thick = option.rect.height() / 2;
        int ym = option.rect.y() + ((option.rect.height() - thick) / 2);

        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);
        painter->setBrush(textColor);

        //! draw ending cirlce
        painter->drawEllipse(QPoint(xm, ym + thick/2), thick/4, thick/4);
    }
}

void Shared::updateButtonText(QWidget *editor, const QModelIndex &index) const
{
    if (!editor) {
        return;
    }

    Data::LayoutsTable allLayouts = qvariant_cast<Data::LayoutsTable>(index.data(Model::Layouts::ALLLAYOUTSROLE));

    QPushButton *button = static_cast<QPushButton *>(editor);
    Data::LayoutsTable assignedLayouts;

    foreach (QAction *action, button->menu()->actions()) {
        if (action->isChecked()) {
            QString id = action->data().toString();
            if (allLayouts.containsId(id)) {
                assignedLayouts << allLayouts[action->data().toString()];
            }
        }
    }

    button->setText(joined(assignedLayouts, false));
}

QString Shared::joined(const Data::LayoutsTable &layouts, bool formatText) const
{
    QString finalText;

    for (int i = 0; i < layouts.rowCount(); ++i) {
        if (i > 0) {
            finalText += ", ";
        }

        bool bold {false};

        if (formatText && layouts[i].isActive) {
            bold = true;
        }

        finalText += bold ? "<b>" + layouts[i].name + "</b>" : layouts[i].name;
    }

    return finalText;
}

}
}
}
}
