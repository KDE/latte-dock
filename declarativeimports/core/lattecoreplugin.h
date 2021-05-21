/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTECOREPLUGIN_H
#define LATTECOREPLUGIN_H

// Qt
#include <QQmlExtensionPlugin>

class LatteCorePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif
