/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionshandler.h"

// local
#include "ui_actionsdialog.h"
#include "actionsdialog.h"
#include "../../data/contextmenudata.h"

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Handler {

ActionsHandler::ActionsHandler(Dialog::ActionsDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui())
{
    init();
    //c_alwaysActions = table(currentActions);
}

ActionsHandler::~ActionsHandler()
{
}

void ActionsHandler::init()
{
    def_alwaysActions = table(Data::ContextMenu::ACTIONSALWAYSVISIBLE);
}

bool ActionsHandler::hasChangedData() const
{
    return c_alwaysActions != c_alwaysActions;
}

bool ActionsHandler::inDefaultValues() const
{
    return c_alwaysActions == def_alwaysActions;
}

Data::GenericTable<Data::Generic> ActionsHandler::table(const QStringList &ids)
{
    Data::GenericTable<Data::Generic> bastable;

    for(int i=0; i<ids.count(); ++i) {
        bastable << Data::Generic(ids[i], "");
    }

    return bastable;
}

QStringList ActionsHandler::currentData() const
{
    return c_alwaysActions.ids();
}

void ActionsHandler::reset()
{
    c_alwaysActions = o_alwaysActions;
}

void ActionsHandler::resetDefaults()
{
    c_alwaysActions = def_alwaysActions;
}

void ActionsHandler::save()
{
    //do nothing
}

}
}
}
