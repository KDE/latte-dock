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
#include "../viewsmodel.h"
#include "../../../data/genericbasictable.h"

// Qt
#include <QMenu>
#include <QPushButton>

namespace Latte {
namespace Settings {
namespace View {
namespace Delegate {

SingleOption::SingleOption(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *SingleOption::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPushButton *button = new QPushButton(parent);

    QMenu *menu = new QMenu(button);
    button->setMenu(menu);
    menu->setMinimumWidth(option.rect.width());

    bool isViewActive = index.data(Model::Views::ISACTIVEROLE).toBool();

    QString currentChoice = index.data(Qt::UserRole).toString();
    Latte::Data::GenericBasicTable choices = index.data(Model::Views::CHOICESROLE).value<Latte::Data::GenericBasicTable>();

    for (int i=0; i<choices.rowCount(); ++i) {
        QAction *action = new QAction(choices[i].name);
        action->setData(choices[i].id);

        if (choices[i].id == currentChoice) {
            action->setIcon(QIcon::fromTheme("dialog-yes"));
        }

        menu->addAction(action);
    }

    return button;
}

}
}
}
}
