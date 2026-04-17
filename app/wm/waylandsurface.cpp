/*
    SPDX-FileCopyrightText: 2026 OpenAI
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "waylandsurface.h"

// Qt
#include <QWindow>
#include <QDebug>

#include <QtWaylandClient/QWaylandClientExtension>
#include <QtWaylandClient/private/qwaylandwindow_p.h>

// KDE
#include <LayerShellQt/Window>
#include <PlasmaQuick/PlasmaShellWaylandIntegration>

// Generated during the build.
#include "qwayland-plasma-shell.h"

namespace {

class PlasmaShellExtension : public QWaylandClientExtensionTemplate<PlasmaShellExtension>, public QtWayland::org_kde_plasma_shell
{
public:
    PlasmaShellExtension()
        : QWaylandClientExtensionTemplate<PlasmaShellExtension>(8)
    {
        initialize();
    }

    org_kde_plasma_surface *createSurface(struct ::wl_surface *surface)
    {
        if (!isActive() || !surface) {
            return nullptr;
        }

        return get_surface(surface);
    }
};

PlasmaShellExtension *plasmaShellExtension()
{
    static PlasmaShellExtension s_extension;
    return &s_extension;
}

const org_kde_plasma_surface_listener s_listener = {
    .auto_hidden_panel_hidden = Latte::WindowSystem::WaylandSurface::autoHidePanelHiddenCallback,
    .auto_hidden_panel_shown = Latte::WindowSystem::WaylandSurface::autoHidePanelShownCallback,
};

}

namespace Latte {
namespace WindowSystem {

WaylandSurface::WaylandSurface(QWindow *window, QObject *parent)
    : QObject(parent),
      m_window(window)
{
    if (m_window) {
        m_plasmaIntegration = PlasmaShellWaylandIntegration::get(m_window);
    }
}

WaylandSurface::~WaylandSurface()
{
    release();
}

bool WaylandSurface::isReady() const
{
    return m_surface != nullptr;
}

bool WaylandSurface::sync()
{
    if (m_surface || !m_window) {
        return m_surface != nullptr;
    }

    if (!m_plasmaIntegration) {
        m_plasmaIntegration = PlasmaShellWaylandIntegration::get(m_window);
    }

    auto *platformWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(m_window->handle());

    if (!platformWindow || !platformWindow->wlSurface()) {
        return false;
    }

    m_surface = plasmaShellExtension()->createSurface(platformWindow->wlSurface());

    if (!m_surface) {
        return false;
    }

    org_kde_plasma_surface_add_listener(m_surface, &s_listener, this);
    applyState();
    emit readyChanged();
    return true;
}

void WaylandSurface::release()
{
    if (!m_surface) {
        return;
    }

    org_kde_plasma_surface_destroy(m_surface);
    m_surface = nullptr;
    emit readyChanged();
}

void WaylandSurface::setPosition(const QPoint &position)
{
    m_position = position;
    m_hasPosition = true;

    if (m_plasmaIntegration) {
        m_plasmaIntegration->setPosition(position);
    }

    sync();
}

void WaylandSurface::setRole(Role role)
{
    m_role = role;

    uint32_t wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_NORMAL;

    switch (role) {
    case Role::Normal:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_NORMAL;
        break;
    case Role::Panel:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_PANEL;
        break;
    case Role::ToolTip:
        wlRole = ORG_KDE_PLASMA_SURFACE_ROLE_TOOLTIP;
        break;
    }

    if (m_plasmaIntegration) {
        m_plasmaIntegration->setRole(static_cast<QtWayland::org_kde_plasma_surface::role>(wlRole));
    }

    sync();
}

void WaylandSurface::setPanelBehavior(PanelBehavior behavior)
{
    m_panelBehavior = behavior;

    uint32_t wlBehavior = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_ALWAYS_VISIBLE;

    switch (behavior) {
    case PanelBehavior::AlwaysVisible:
        wlBehavior = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_ALWAYS_VISIBLE;
        break;
    case PanelBehavior::AutoHide:
        wlBehavior = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_AUTO_HIDE;
        break;
    case PanelBehavior::WindowsCanCover:
        wlBehavior = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_WINDOWS_CAN_COVER;
        break;
    case PanelBehavior::WindowsGoBelow:
        wlBehavior = ORG_KDE_PLASMA_SURFACE_PANEL_BEHAVIOR_WINDOWS_GO_BELOW;
        break;
    }

    if (m_plasmaIntegration) {
        m_plasmaIntegration->setPanelBehavior(static_cast<QtWayland::org_kde_plasma_surface::panel_behavior>(wlBehavior));
    }

    sync();
}

void WaylandSurface::setPanelTakesFocus(bool takesFocus)
{
    m_panelTakesFocus = takesFocus;

    if (m_plasmaIntegration) {
        m_plasmaIntegration->setTakesFocus(takesFocus);
    }

    sync();
}

void WaylandSurface::setSkipTaskbar(bool skip)
{
    m_skipTaskbar = skip;

    if (sync()) {
        org_kde_plasma_surface_set_skip_taskbar(m_surface, skip);
    }
}

void WaylandSurface::setSkipSwitcher(bool skip)
{
    m_skipSwitcher = skip;

    if (sync()) {
        org_kde_plasma_surface_set_skip_switcher(m_surface, skip);
    }
}

void WaylandSurface::requestHideAutoHidingPanel()
{
    if (sync()) {
        org_kde_plasma_surface_panel_auto_hide_hide(m_surface);
    }
}

void WaylandSurface::requestShowAutoHidingPanel()
{
    if (sync()) {
        org_kde_plasma_surface_panel_auto_hide_show(m_surface);
    }
}

LayerShellQt::Window *WaylandSurface::layerWindow()
{
    if (!m_layerWindow && m_window) {
        m_layerWindow = LayerShellQt::Window::get(m_window);
    }

    return m_layerWindow;
}

void WaylandSurface::applyState()
{
    setRole(m_role);
    setPanelBehavior(m_panelBehavior);
    setPanelTakesFocus(m_panelTakesFocus);
    setSkipTaskbar(m_skipTaskbar);
    setSkipSwitcher(m_skipSwitcher);

    if (m_hasPosition) {
        setPosition(m_position);
    }
}

void WaylandSurface::autoHidePanelHiddenCallback(void *data, org_kde_plasma_surface *surface)
{
    auto *self = reinterpret_cast<WaylandSurface *>(data);

    if (!self || self->m_surface != surface) {
        return;
    }

    emit self->autoHidePanelHidden();
}

void WaylandSurface::autoHidePanelShownCallback(void *data, org_kde_plasma_surface *surface)
{
    auto *self = reinterpret_cast<WaylandSurface *>(data);

    if (!self || self->m_surface != surface) {
        return;
    }

    emit self->autoHidePanelShown();
}

}
}
