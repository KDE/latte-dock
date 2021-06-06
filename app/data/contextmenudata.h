/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONTEXTMENUDATA_H
#define CONTEXTMENUDATA_H

// Qt
#include <QStringList>

namespace Latte {
namespace Data {
namespace ContextMenu {

static const char ADDVIEWACTION[]= "_add_view";
static const char LAYOUTSACTION[] = "_layouts";
static const char PREFERENCESACTION[] = "_preferences";
static const char QUITLATTEACTION[] = "_quit_latte";
static const char ADDWIDGETSACTION[] = "_add_latte_widgets";
static const char DUPLICATEVIEWACTION[] = "_duplicate_view";
static const char EDITVIEWACTION[] = "_edit_view";
static const char EXPORTVIEWTEMPLATEACTION[] = "_export_view";
static const char REMOVEVIEWACTION[] = "_remove_view";
static const char MOVEVIEWACTION[] = "_move_view";
static const char SEPARATOR1ACTION[] = "_separator1";

static QStringList ACTIONSALWAYSVISIBLE = {LAYOUTSACTION,
                                           PREFERENCESACTION,
                                           QUITLATTEACTION,
                                           SEPARATOR1ACTION,
                                           ADDWIDGETSACTION,
                                           ADDVIEWACTION,
                                           MOVEVIEWACTION,
                                           EXPORTVIEWTEMPLATEACTION,
                                           REMOVEVIEWACTION};

static QStringList ACTIONSVISIBLEONLYINEDIT = {MOVEVIEWACTION,
                                               EXPORTVIEWTEMPLATEACTION,
                                               REMOVEVIEWACTION};

}
}
}

#endif
