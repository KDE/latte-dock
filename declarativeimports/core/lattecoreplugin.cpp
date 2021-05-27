/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lattecoreplugin.h"

// local
#include "dialog.h"
#include "environment.h"
#include "iconitem.h"
#include "quickwindowsystem.h"
#include "tools.h"

#include <types.h>

// Qt
#include <QtQml>


void LatteCorePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.latte.core"));
    qmlRegisterUncreatableType<Latte::Types>(uri, 0, 2, "Types", "Latte Types uncreatable");
    qmlRegisterType<Latte::IconItem>(uri, 0, 2, "IconItem");
    qmlRegisterType<Latte::Quick::Dialog>(uri, 0, 2, "Dialog");
    qmlRegisterSingletonType<Latte::Environment>(uri, 0, 2, "Environment", &Latte::environment_qobject_singletontype_provider);
    qmlRegisterSingletonType<Latte::Tools>(uri, 0, 2, "Tools", &Latte::tools_qobject_singletontype_provider);
    qmlRegisterSingletonType<Latte::QuickWindowSystem>(uri, 0, 2, "WindowSystem", &Latte::windowsystem_qobject_singletontype_provider);
}
