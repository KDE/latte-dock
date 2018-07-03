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
#include "visibilitymanager.h"
#include "../layout.h"
#include "../plasmaquick/containmentview.h"
#include "../plasmaquick/configview.h"
#include "../../liblattedock/dock.h"

#include <QQuickView>
#include <QQmlListProperty>
#include <QMenu>
#include <QMimeData>
#include <QScreen>
#include <QPointer>
#include <QTimer>

#include <Plasma/Theme>

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
    Q_PROPERTY(bool blockAnimations READ blockAnimations WRITE setBlockAnimations NOTIFY blockAnimationsChanged)
    Q_PROPERTY(bool colorizerSupport READ colorizerSupport WRITE setColorizerSupport NOTIFY colorizerSupportChanged)
    Q_PROPERTY(bool contextMenuIsShown READ contextMenuIsShown NOTIFY contextMenuIsShownChanged)
    Q_PROPERTY(bool dockWinBehavior READ dockWinBehavior WRITE setDockWinBehavior NOTIFY dockWinBehaviorChanged)
    Q_PROPERTY(bool drawShadows READ drawShadows WRITE setDrawShadows NOTIFY drawShadowsChanged)
    Q_PROPERTY(bool drawEffects READ drawEffects WRITE setDrawEffects NOTIFY drawEffectsChanged)
    //! Because Latte uses animations, changing to edit mode it may be different than
    //! when the isUserConfiguring changes value
    Q_PROPERTY(bool inEditMode READ inEditMode WRITE setInEditMode NOTIFY inEditModeChanged)
    Q_PROPERTY(bool onPrimary READ onPrimary WRITE setOnPrimary NOTIFY onPrimaryChanged)

    Q_PROPERTY(int alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(int docksCount READ docksCount NOTIFY docksCountChanged)
    Q_PROPERTY(int dockTransparency READ dockTransparency WRITE setDockTransparency NOTIFY dockTransparencyChanged)
    Q_PROPERTY(int fontPixelSize READ fontPixelSize WRITE setFontPixelSize NOTIFY fontPixelSizeChanged)
    Q_PROPERTY(int totalDocksCount READ totalDocksCount NOTIFY totalDocksCountChanged)
    Q_PROPERTY(int x READ x NOTIFY xChanged)
    Q_PROPERTY(int y READ y NOTIFY yChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int maxThickness READ maxThickness WRITE setMaxThickness NOTIFY maxThicknessChanged)
    Q_PROPERTY(int normalThickness READ normalThickness WRITE setNormalThickness NOTIFY normalThicknessChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(int shadow READ shadow WRITE setShadow NOTIFY shadowChanged)

    Q_PROPERTY(QString currentScreen READ currentScreen NOTIFY currentScreenChanged)

    Q_PROPERTY(float maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)

    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders NOTIFY enabledBordersChanged)

    Q_PROPERTY(VisibilityManager *visibility READ visibility NOTIFY visibilityChanged)
    Q_PROPERTY(Layout *managedLayout READ managedLayout WRITE setManagedLayout NOTIFY managedLayoutChanged)
    Q_PROPERTY(QQmlListProperty<QScreen> screens READ screens)

    Q_PROPERTY(QRect effectsArea READ effectsArea WRITE setEffectsArea NOTIFY effectsAreaChanged)
    Q_PROPERTY(QRect absoluteGeometry READ absGeometry NOTIFY absGeometryChanged)
    Q_PROPERTY(QRect localGeometry READ localGeometry WRITE setLocalGeometry NOTIFY localGeometryChanged)
    Q_PROPERTY(QRect maskArea READ maskArea WRITE setMaskArea NOTIFY maskAreaChanged)
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    DockView(Plasma::Corona *corona, QScreen *targetScreen = nullptr, bool dockWindowBehavior = false);
    virtual ~DockView();

    void init();

    void setScreenToFollow(QScreen *screen, bool updateScreenId = true);

    void resizeWindow(QRect availableScreenRect = QRect());
    void syncGeometry();

    bool alternativesIsShown() const;
    void setAlternativesIsShown(bool show);

    bool onPrimary() const;
    void setOnPrimary(bool flag);

    int currentThickness() const;

    int docksCount() const;
    int totalDocksCount() const;

    bool behaveAsPlasmaPanel() const;
    void setBehaveAsPlasmaPanel(bool behavior);

    bool blockAnimations() const;
    void setBlockAnimations(bool block);

    bool colorizerSupport() const;
    void setColorizerSupport(bool support);

    bool contextMenuIsShown() const;

    bool dockWinBehavior() const;
    void setDockWinBehavior(bool dock);

    bool drawShadows() const;
    void setDrawShadows(bool draw);

    bool drawEffects() const;
    void setDrawEffects(bool draw);

    bool inEditMode() const;
    void setInEditMode(bool edit);

    float maxLength() const;
    void setMaxLength(float length);

    int dockTransparency() const;
    void setDockTransparency(int transparency);

    int fontPixelSize() const;
    void setFontPixelSize(int size);

    int maxThickness() const;
    void setMaxThickness(int thickness);

    int normalThickness() const;
    void setNormalThickness(int thickness);

    int offset() const;
    void setOffset(int offset);

    int shadow() const;
    void setShadow(int shadow);

    int alignment() const;
    void setAlignment(int alignment);

    QRect maskArea() const;
    void setMaskArea(QRect area);

    QRect effectsArea() const;
    void setEffectsArea(QRect area);

    QRect absGeometry() const;
    QRect screenGeometry() const;

    bool inLocationChangeAnimation();

    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    QString currentScreen() const;

    QRect localGeometry() const;
    void setLocalGeometry(const QRect &geometry);

    bool settingsWindowIsShown();
    void showSettingsWindow();

    VisibilityManager *visibility() const;

    Layout *managedLayout() const;
    void setManagedLayout(Layout *layout);

    KWayland::Client::PlasmaShellSurface *surface();

    QQmlListProperty<QScreen> screens();
    static int countScreens(QQmlListProperty<QScreen> *property);
    static QScreen *atScreens(QQmlListProperty<QScreen> *property, int index);
    void reconsiderScreen();

    //! these are signals that create crashes, such a example is the availableScreenRectChanged from corona
    //! when its containment is destroyed
    void disconnectSensitiveSignals();

public slots:
    Q_INVOKABLE void addNewDock();
    Q_INVOKABLE void removeDock();
    Q_INVOKABLE void copyDock();

    Q_INVOKABLE QList<int> freeEdges() const;
    Q_INVOKABLE QVariantList containmentActions();

    Q_INVOKABLE void deactivateApplets();
    Q_INVOKABLE void moveToLayout(QString layoutName);
    Q_INVOKABLE void removeTasksPlasmoid();
    Q_INVOKABLE void setBlockHiding(bool block);
    Q_INVOKABLE void toggleAppletExpanded(const int id);
    Q_INVOKABLE void updateEnabledBorders();

    Q_INVOKABLE void hideDockDuringLocationChange(int goToLocation);
    Q_INVOKABLE void hideDockDuringMovingToLayout(QString layoutName);

    Q_INVOKABLE int docksWithTasks();

    Q_INVOKABLE bool mimeContainsPlasmoid(QMimeData *mimeData, QString name);
    Q_INVOKABLE bool setCurrentScreen(const QString id);
    Q_INVOKABLE bool tasksPresent();
    Q_INVOKABLE bool latteTasksPresent();

    void updateAbsDockGeometry(bool bypassChecks = false);

protected slots:
    void showConfigurationInterface(Plasma::Applet *applet) override;

protected:
    bool event(QEvent *ev) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void addInternalViewSplitter();
    void removeInternalViewSplitter();
    void eventTriggered(QEvent *ev);

    //! these two signals are used from config ui and containment ui
    //! in order to orchestrate an animated hiding/showing of dock
    //! during changing location
    void hideDockDuringLocationChangeStarted();
    void hideDockDuringLocationChangeFinished();
    void hideDockDuringScreenChangeStarted();
    void hideDockDuringScreenChangeFinished();
    void hideDockDuringMovingToLayoutStarted();
    void hideDockDuringMovingToLayoutFinished();
    void showDockAfterLocationChangeFinished();
    void showDockAfterScreenChangeFinished();
    void showDockAfterMovingToLayoutFinished();

    void activitiesChanged();
    void alternativesIsShownChanged();
    void alignmentChanged();
    void behaveAsPlasmaPanelChanged();
    void blockAnimationsChanged();
    void colorizerSupportChanged();
    void contextMenuIsShownChanged();
    void currentScreenChanged();
    void dockLocationChanged();
    void docksCountChanged();
    void dockTransparencyChanged();
    void dockWinBehaviorChanged();
    void drawShadowsChanged();
    void drawEffectsChanged();
    void effectsAreaChanged();
    void enabledBordersChanged();
    void fontPixelSizeChanged();
    void widthChanged();
    void heightChanged();
    void inEditModeChanged();
    void localGeometryChanged();
    void managedLayoutChanged();
    void maxLengthChanged();
    void maxThicknessChanged();
    void normalThicknessChanged();
    void offsetChanged();
    void onPrimaryChanged();
    void visibilityChanged();
    void maskAreaChanged();
    void screenGeometryChanged();
    void shadowChanged();
    void themeChanged();
    void totalDocksCountChanged();
    void xChanged();
    void yChanged();

    void absGeometryChanged(const QRect &geometry);

private slots:
    void availableScreenRectChanged();
    void hideWindowsForSlidingOut();
    void statusChanged(Plasma::Types::ItemStatus);
    void screenChanged(QScreen *screen);
    void updateEffects();
    void validateDockGeometry();

    void restoreConfig();
    void saveConfig();

private:
    void applyActivitiesToWindows();
    void initSignalingForLocationChangeSliding();
    void setupWaylandIntegration();
    void updatePosition(QRect availableScreenRect = QRect());
    void updateFormFactor();
    void updateAppletContainsMethod();

    QRect maximumNormalGeometry();

private:
    Plasma::Containment *containmentById(uint id);

    bool m_alternativesIsShown{false};
    bool m_behaveAsPlasmaPanel{false};
    bool m_blockAnimations{false};
    bool m_colorizerSupport{false};
    bool m_dockWinBehavior{true};
    bool m_drawShadows{true};
    bool m_drawEffects{false};
    bool m_forceDrawCenteredBorders{false};
    bool m_inDelete{false};
    bool m_inEditMode{false};
    bool m_onPrimary{true};
    int m_dockTransparency{100};
    int m_fontPixelSize{ -1};
    int m_maxThickness{24};
    int m_normalThickness{24};
    int m_offset{0};
    int m_shadow{0};
    float m_maxLength{1};

    Dock::Alignment m_alignment{Dock::Center};

    QRect m_effectsArea;
    QRect m_localGeometry;
    QRect m_absGeometry;
    QRect m_maskArea;
    //! it is used in order to enforce X11 to never miss window geometry
    QRect m_validGeometry;

    Layout *m_managedLayout{nullptr};
    QPointer<PlasmaQuick::ConfigView> m_configView;

    QPointer<VisibilityManager> m_visibility;
    QPointer<DockMenuManager> m_menuManager;
    QPointer<QScreen> m_screenToFollow;

    QString m_screenToFollowId;

    QTimer m_screenSyncTimer;
    QTimer m_validateGeometryTimer;

    //! Connections to release and bound for the managed layout
    std::array<QMetaObject::Connection, 4> connectionsManagedLayout;

    //!used at sliding out/in animation
    QString m_moveToLayout;
    Plasma::Types::Location m_goToLocation{Plasma::Types::Floating};
    QScreen *m_goToScreen{nullptr};

    Plasma::Theme m_theme;
    //only for the mask on disabled compositing, not to actually paint
    Plasma::FrameSvg *m_background{nullptr};

    //only for the mask, not to actually paint
    Plasma::FrameSvg::EnabledBorders m_enabledBorders{Plasma::FrameSvg::AllBorders};
    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}

#endif
