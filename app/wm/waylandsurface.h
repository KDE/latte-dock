/*
    SPDX-FileCopyrightText: 2026 OpenAI
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTE_WAYLANDSURFACE_H
#define LATTE_WAYLANDSURFACE_H

// Qt
#include <QObject>
#include <QPoint>
#include <QPointer>

namespace LayerShellQt {
class Window;
}

class PlasmaShellWaylandIntegration;
class QWindow;
struct org_kde_plasma_surface;

namespace Latte {
namespace WindowSystem {

class WaylandSurface : public QObject
{
    Q_OBJECT

public:
    enum class Role {
        Normal,
        Panel,
        ToolTip
    };

    enum class PanelBehavior {
        AlwaysVisible,
        AutoHide,
        WindowsCanCover,
        WindowsGoBelow
    };

    explicit WaylandSurface(QWindow *window, QObject *parent = nullptr);
    ~WaylandSurface() override;

    bool isReady() const;
    bool sync();
    void release();

    void setPosition(const QPoint &position);
    void setRole(Role role);
    void setPanelBehavior(PanelBehavior behavior);
    void setPanelTakesFocus(bool takesFocus);
    void setSkipTaskbar(bool skip);
    void setSkipSwitcher(bool skip);

    void requestHideAutoHidingPanel();
    void requestShowAutoHidingPanel();

    LayerShellQt::Window *layerWindow();

    static void autoHidePanelHiddenCallback(void *data, org_kde_plasma_surface *surface);
    static void autoHidePanelShownCallback(void *data, org_kde_plasma_surface *surface);

signals:
    void readyChanged();
    void autoHidePanelHidden();
    void autoHidePanelShown();

private:
    void applyState();

private:
    QPointer<QWindow> m_window;
    org_kde_plasma_surface *m_surface{nullptr};
    LayerShellQt::Window *m_layerWindow{nullptr};
    PlasmaShellWaylandIntegration *m_plasmaIntegration{nullptr};

    QPoint m_position;
    Role m_role{Role::Normal};
    PanelBehavior m_panelBehavior{PanelBehavior::AlwaysVisible};
    bool m_panelTakesFocus{false};
    bool m_skipTaskbar{false};
    bool m_skipSwitcher{false};
    bool m_hasPosition{false};
};

}
}

#endif
