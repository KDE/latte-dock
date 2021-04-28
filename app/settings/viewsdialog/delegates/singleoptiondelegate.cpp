/*
*  Copyright 2021 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "singleoptiondelegate.h"

// local
#include "custommenuitemwidget.h"
#include "../viewsmodel.h"
#include "../../generic/generictools.h"
#include "../../../data/genericbasictable.h"
#include "../../../data/screendata.h"

// Qt
#include <QApplication>
#include <QMenu>
#include <QPushButton>
#include <QTextDocument>
#include <QWidgetAction>

#define PRESSEDPROPERTY "PRESSED"

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

SingleOption::SingleOption(QObject *parent)
    : SingleText(parent)
{
}

QWidget *SingleOption::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const int row = index.row();
    const int column = index.column();

    QPushButton *button = new QPushButton(parent);

    QMenu *menu = new QMenu(button);
    button->setMenu(menu);
    menu->setMinimumWidth(option.rect.width());

    bool isViewActive = index.data(Model::Views::ISACTIVEROLE).toBool();

    QString currentChoice = index.data(Qt::UserRole).toString();

    Latte::Data::GenericBasicTable choices;
    QStringList activeChoices;

    Latte::Data::ScreensTable screens;

    if (column == Model::Views::SCREENCOLUMN) {
        screens = index.data(Model::Views::CHOICESROLE).value<Latte::Data::ScreensTable>();

        for (int i=0; i<screens.rowCount(); ++i) {
            choices << Latte::Data::Generic(screens[i].id, screens[i].name);

            if (screens[i].isActive) {
                activeChoices << screens[i].id;
            }
        }
    } else {
        choices << index.data(Model::Views::CHOICESROLE).value<Latte::Data::GenericBasicTable>();
    }

    for (int i=0; i<choices.rowCount(); ++i) {
        QWidgetAction *action = new QWidgetAction(menu);
        action->setText(choices[i].name);
        action->setData(choices[i].id);

        if (choices[i].id == currentChoice) {
            action->setIcon(QIcon::fromTheme("dialog-yes"));
        }

        if (activeChoices.contains(choices[i].id)) {
            QFont font = action->font();
            font.setBold(true);
            action->setFont(font);
        }

        connect(action, &QAction::triggered, this, [this, button, menu, action](bool checked) {
            menu->setProperty(PRESSEDPROPERTY, action->data());
            button->clearFocus();
        });

        Settings::View::Widget::CustomMenuItemWidget *optioncustomwidget = new Settings::View::Widget::CustomMenuItemWidget(action, menu);

        if (column == Model::Views::SCREENCOLUMN) {
            optioncustomwidget->setScreen(screens[i]);
        }

        action->setDefaultWidget(optioncustomwidget);
        menu->addAction(action);
    }

    return button;
}

void SingleOption::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

void SingleOption::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    updateButton(editor, index.data(Qt::DisplayRole).toString());
}

bool SingleOption::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                               const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void SingleOption::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QPushButton *button = static_cast<QPushButton *>(editor);
    QMenu *menu = button->menu();

    if (menu->property(PRESSEDPROPERTY).toString().isEmpty()) {
        return;
    }

    model->setData(index, menu->property(PRESSEDPROPERTY), Qt::UserRole);
    updateButton(editor, index.data(Qt::DisplayRole).toString());
}

void SingleOption::updateButton(QWidget *editor, const QString &text) const
{
    QPushButton *button = static_cast<QPushButton *>(editor);
    button->setText(text);
}

}
}
}
}
