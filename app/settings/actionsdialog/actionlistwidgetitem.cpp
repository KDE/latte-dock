/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionlistwidgetitem.h"


namespace Latte {
namespace Settings {
namespace ActionsDialog {

ActionListWidgetItem::ActionListWidgetItem(const QIcon &icon, const QString &text, const int &order, const QString &id, QListWidget *parent, int type)
    : QListWidgetItem(icon, text, parent, type)
{
    setData(IDROLE, id);
    setData(ORDERROLE, order);
}

bool ActionListWidgetItem::operator<(const QListWidgetItem &other) const
{
    int curorder = data(ORDERROLE).toInt();
    int othorder = other.data(ORDERROLE).toInt();

    return (curorder < othorder);
}

}
}
}
