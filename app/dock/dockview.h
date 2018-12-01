/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef DOCKVIEW_H
#define DOCKVIEW_H

#include "dockconfigview.h"
#include "effects.h"
#include "positioner.h"
#include "visibilitymanager.h"
#include "../layout.h"
#include "../plasmaquick/containmentview.h"
#include "../plasmaquick/configview.h"
#include "../../liblattedock/dock.h"

#include <array>

#include <QQuickView>
#include <QMenu>
#include <QMimeData>
#include <QScreen>
#include <QPointer>
#include <QTimer>

namespace Plasma {
class Types;
class Corona;
class Containment;
}

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Latte {
class DockMenuManager;
class Layout;
}

namespace Latte {

class DockView : public PlasmaQuick::ContainmentView
{
    Q_OBJECT
    Q_PROPERTY(bool alternativesIsShown READ alternativesIsShown NOTIFY alternativesIsShownChanged)
    Q_PROPERTY(bool behaveAsPlasmaPanel READ behaveAsPlasmaPanel WRITE setBehaveAsPlasmaPanel NOTIFY behaveAsPlasmaPanelChanged)
    Q_PROPERTY(bool contextMenuIsShown READ contextMenuIsShown NOTIFY contextMenuIsShownChanged)
    Q_PROPERTY(bool dockWinBehavior READ dockWinBehavior WRITE setDockWinBehavior NOTIFY dockWinBehaviorChanged)
    //! Because Latte uses animations, changing to edit mode it may be different than
    //! when the isUserConfiguring changes value
    Q_PROPERTY(bool inEditMode READ inEditMode WRITE setInEditMode NOTIFY inEditModeChanged)
    Q_PROPERTY(bool isPreferredForShortcuts READ isPreferredForShortcuts WRITE setIsPreferredForShortcuts NOTIFY isPreferredForShortcutsChanged)
    Q_PROPERTY(bool onPrimary READ onPrimary WRITE setOnPrimary NOTIFY onPrimaryChanged)

    Q_PROPERTY(int alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(int fontPixelSize READ fontPixelSize WRITE setFontPixelSize NOTIFY fontPixelSizeChanged)
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int maxThickness READ maxThickness WRITE setMaxThickness NOTIFY maxThicknessChanged)
    Q_PROPERTY(int normalThickness READ normalThickness WRITE setNormalThickness NOTIFY normalThicknessChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)

    Q_PROPERTY(float maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)

    Q_PROPERTY(Latte::View::Effects *effects READ effects NOTIFY effectsChanged)
    Q_PROPERTY(Layout *managedLayout READ managedLayout WRITE setManagedLayout NOTIFY managedLayoutChanged)
    Q_PROPERTY(Latte::View::Positioner *positioner READ positioner NOTIFY positionerChanged)
    Q_PROPERTY(VisibilityManager *visibility READ visibility NOTIFY visibilityChanged)

    Q_PROPERTY(QRect absoluteGeometry READ absGeometry NOTIFY absGeometryChanged)
    Q_PROPERTY(QRect localGeometry READ localGeometry WRITE setLocalGeometry NOTIFY localGeometryChanged)
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    DockView(Plasma::Corona *corona, QScreen *targetScreen = nullptr, bool dockWindowBehavior = false);
    virtual ~DockView();

    void init();

    bool alternativesIsShown() const;
    void setAlternativesIsShown(bool show);

    bool inDelete() const;

    bool onPrimary() const;
    void setOnPrimary(bool flag);

    int currentThickness() const;

    bool behaveAsPlasmaPanel() const;
    void setBehaveAsPlasmaPanel(bool behavior);

    bool contextMenuIsShown() const;

    bool dockWinBehavior() const;
    void setDockWinBehavior(bool dock);

    bool inEditMode() const;
    void setInEditMode(bool edit);

    bool isPreferredForShortcuts() const;
    void setIsPreferredForShortcuts(bool preferred);

    float maxLength() const;
    void setMaxLength(float length);

    int fontPixelSize() const;
    void setFontPixelSize(int size);

    int maxThickness() const;
    void setMaxThickness(int thickness);

    int normalThickness() const;
    void setNormalThickness(int thickness);

    int offset() const;
    void setOffset(int offset);

    int alignment() const;
    void setAlignment(int alignment);

    QRect absGeometry() const;
    QRect screenGeometry() const;

    QRect localGeometry() const;
    void setLocalGeometry(const QRect &geometry);

    bool settingsWindowIsShown();
    void showSettingsWindow();

    View::Effects *effects() const;
    View::Positioner *positioner() const;
    VisibilityManager *visibility() const;

    Layout *managedLayout() const;
    void setManagedLayout(Layout *layout);

    KWayland::Client::PlasmaShellSurface *surface();

    void reconsiderScreen();

    //! these are signals that create crashes, such a example is the availableScreenRectChanged from corona
    //! when its containment is destroyed
    void disconnectSensitiveSignals();

public slots:
    Q_INVOKABLE void addNewDock();
    Q_INVOKABLE void removeDock();
    Q_INVOKABLE void copyDock();

    Q_INVOKABLE QVariantList containmentActions();

    Q_INVOKABLE void deactivateApplets();
    Q_INVOKABLE void moveToLayout(QString layoutName);
    Q_INVOKABLE void removeTasksPlasmoid();
    Q_INVOKABLE void setBlockHiding(bool block);
    Q_INVOKABLE void toggleAppletExpanded(const int id);

    Q_INVOKABLE bool mimeContainsPlasmoid(QMimeData *mimeData, QString name);
    Q_INVOKABLE bool tasksPresent();
    Q_INVOKABLE bool latteTasksPresent();

    void updateAbsDockGeometry(bool bypassChecks = false);

    Q_INVOKABLE void disableGrabItemBehavior();
    Q_INVOKABLE void restoreGrabItemBehavior();

protected slots:
    void showConfigurationInterface(Plasma::Applet *applet) override;

protected:
    bool event(QEvent *ev) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void addInternalViewSplitter();
    void removeInternalViewSplitter();
    void eventTriggered(QEvent *ev);

    void activitiesChanged();
    void alternativesIsShownChanged();
    void alignmentChanged();
    void behaveAsPlasmaPanelChanged();
    void contextMenuIsShownChanged();
    void dockLocationChanged();
    void dockWinBehaviorChanged();
    void effectsChanged();
    void fontPixelSizeChanged();
    void widthChanged();
    void heightChanged();
    void inEditModeChanged();
    void isPreferredForShortcutsChanged();
    void localGeometryChanged();
    void managedLayoutChanged();
    void maxLengthChanged();
    void maxThicknessChanged();
    void normalThicknessChanged();
    void offsetChanged();
    void onPrimaryChanged();
    void visibilityChanged();
    void positionerChanged();
    void screenGeometryChanged();
    void xChanged();
    void yChanged();

    void absGeometryChanged(const QRect &geometry);

private slots:
    void availableScreenRectChanged();
    void hideWindowsForSlidingOut();
    void preferredViewForShortcutsChangedSlot(DockView *view);
    void statusChanged(Plasma::Types::ItemStatus);

    void restoreConfig();
    void saveConfig();

private:
    void applyActivitiesToWindows();
    void initSignalingForLocationChangeSliding();
    void setupWaylandIntegration();
    void updateAppletContainsMethod();

private:
    Plasma::Containment *containmentById(uint id);

    bool m_alternativesIsShown{false};
    bool m_behaveAsPlasmaPanel{false};
    bool m_dockWinBehavior{true};
    bool m_inDelete{false};
    bool m_inEditMode{false};
    bool m_isPreferredForShortcuts{false};
    bool m_onPrimary{true};
    int m_fontPixelSize{ -1};
    int m_maxThickness{24};
    int m_normalThickness{24};
    int m_offset{0};
    float m_maxLength{1};

    Dock::Alignment m_alignment{Dock::Center};

    QRect m_localGeometry;
    QRect m_absGeometry;

    Layout *m_managedLayout{nullptr};
    QPointer<PlasmaQuick::ConfigView> m_configView;

    QPointer<DockMenuManager> m_menuManager;
    QPointer<View::Effects> m_effects;
    QPointer<View::Positioner> m_positioner;
    QPointer<VisibilityManager> m_visibility;

    //! Connections to release and bound for the managed layout
    std::array<QMetaObject::Connection, 5> connectionsManagedLayout;

    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}

#endif
