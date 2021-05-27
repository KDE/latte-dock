/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interfaces.h"

#include <PlasmaQuick/AppletQuickItem>

namespace Latte{

Interfaces::Interfaces(QObject *parent)
    : QObject(parent)
{
}

QObject *Interfaces::globalShortcuts() const
{
    return m_globalShortcuts;
}

void Interfaces::setGlobalShortcuts(QObject *shortcuts)
{
    if (m_globalShortcuts == shortcuts) {
        return;
    }

    m_globalShortcuts = shortcuts;

    if (m_globalShortcuts) {
        connect(m_globalShortcuts, &QObject::destroyed, this, [&]() {
            setGlobalShortcuts(nullptr);
        });
    }

    emit globalShortcutsChanged();
}

QObject *Interfaces::layoutsManager() const
{
    return m_layoutsManager;
}

void Interfaces::setLayoutsManager(QObject *manager)
{
    if (m_layoutsManager == manager) {
        return;
    }

    m_layoutsManager = manager;

    if (m_layoutsManager) {
        connect(m_layoutsManager, &QObject::destroyed, this, [&]() {
            setLayoutsManager(nullptr);
        });
    }

    emit layoutsManagerChanged();
}

QObject *Interfaces::themeExtended() const
{
    return m_themeExtended;
}

void Interfaces::setThemeExtended(QObject *theme)
{
    if (m_themeExtended == theme) {
        return;
    }

    m_themeExtended = theme;

    if (m_themeExtended) {
        connect(m_themeExtended, &QObject::destroyed, this, [&]() {
            setThemeExtended(nullptr);
        });
    }

    emit themeExtendedChanged();
}

QObject *Interfaces::universalSettings() const
{
    return m_universalSettings;
}

void Interfaces::setUniversalSettings(QObject *settings)
{
    if (m_universalSettings == settings) {
        return;
    }

    m_universalSettings = settings;

    if (m_universalSettings) {
        connect(m_universalSettings, &QObject::destroyed, this, [&]() {
            setUniversalSettings(nullptr);
        });
    }

    emit universalSettingsChanged();
}

void Interfaces::updateView()
{
    if (m_plasmoid) {
        setView(m_plasmoid->property("_latte_view_object").value<QObject *>());
    }
}

QObject *Interfaces::view() const
{
    return m_view;
}

void Interfaces::setView(QObject *view)
{
    if (m_view == view) {
        return;
    }

    m_view = view;

    if (m_view) {
        connect(m_view, &QObject::destroyed, this, [&]() {
            setView(nullptr);
        });
    }

    emit viewChanged();
}

QObject *Interfaces::plasmoidInterface() const
{
    return m_plasmoid;
}

void Interfaces::setPlasmoidInterface(QObject *interface)
{
    PlasmaQuick::AppletQuickItem *plasmoid = qobject_cast<PlasmaQuick::AppletQuickItem *>(interface);

    if (plasmoid && m_plasmoid != plasmoid) {
        m_plasmoid = plasmoid;

        setGlobalShortcuts(plasmoid->property("_latte_globalShortcuts_object").value<QObject *>());
        setLayoutsManager(plasmoid->property("_latte_layoutsManager_object").value<QObject *>());
        setThemeExtended(plasmoid->property("_latte_themeExtended_object").value<QObject *>());
        setUniversalSettings(plasmoid->property("_latte_universalSettings_object").value<QObject *>());
        setView(plasmoid->property("_latte_view_object").value<QObject *>());

        emit interfaceChanged();
    }
}

}
