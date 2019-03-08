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

#ifndef VIEW_H
#define VIEW_H

// local
#include "effects.h"
#include "positioner.h"
#include "visibilitymanager.h"
#include "windowstracker.h"
#include "settings/primaryconfigview.h"
#include "../shortcuts/globalshortcuts.h"
#include "../layout/layout.h"
#include "../plasma/quick/containmentview.h"
#include "../plasma/quick/configview.h"
#include "../../liblatte2/types.h"

// C++
#include <array>

// Qt
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
class Layout;

namespace ViewPart {
class ContextMenu;
}
}

namespace Latte {

class View : public PlasmaQuick::ContainmentView
{
    Q_OBJECT

    Q_PROPERTY(Latte::Types::ViewType type READ type WRITE setType NOTIFY typeChanged)

    Q_PROPERTY(bool alternativesIsShown READ alternativesIsShown NOTIFY alternativesIsShownChanged)
    Q_PROPERTY(bool behaveAsPlasmaPanel READ behaveAsPlasmaPanel WRITE setBehaveAsPlasmaPanel NOTIFY behaveAsPlasmaPanelChanged)
    Q_PROPERTY(bool byPassWM READ byPassWM WRITE setByPassWM NOTIFY byPassWMChanged)
    Q_PROPERTY(bool contextMenuIsShown READ contextMenuIsShown NOTIFY contextMenuIsShownChanged)
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
    Q_PROPERTY(int editThickness READ editThickness WRITE setEditThickness NOTIFY editThicknessChanged)
    Q_PROPERTY(int maxThickness READ maxThickness WRITE setMaxThickness NOTIFY maxThicknessChanged)
    Q_PROPERTY(int normalThickness READ normalThickness WRITE setNormalThickness NOTIFY normalThicknessChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)

    Q_PROPERTY(float maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)

    Q_PROPERTY(Latte::ViewPart::Effects *effects READ effects NOTIFY effectsChanged)
    Q_PROPERTY(Layout *managedLayout READ managedLayout WRITE setManagedLayout NOTIFY managedLayoutChanged)
    Q_PROPERTY(Latte::ViewPart::Positioner *positioner READ positioner NOTIFY positionerChanged)
    Q_PROPERTY(Latte::ViewPart::VisibilityManager *visibility READ visibility NOTIFY visibilityChanged)
    Q_PROPERTY(Latte::ViewPart::WindowsTracker *windowsTracker READ windowsTracker NOTIFY windowsTrackerChanged)

    Q_PROPERTY(QRect absoluteGeometry READ absGeometry NOTIFY absGeometryChanged)
    Q_PROPERTY(QRect localGeometry READ localGeometry WRITE setLocalGeometry NOTIFY localGeometryChanged)
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    View(Plasma::Corona *corona, QScreen *targetScreen = nullptr, bool byPassWM = false);
    virtual ~View();

    void init();

    Types::ViewType type() const;
    void setType(Types::ViewType type);

    bool alternativesIsShown() const;
    void setAlternativesIsShown(bool show);

    bool inDelete() const;

    bool onPrimary() const;
    void setOnPrimary(bool flag);

    int currentThickness() const;

    bool behaveAsPlasmaPanel() const;
    void setBehaveAsPlasmaPanel(bool behavior);

    bool containsMouse() const;

    bool contextMenuIsShown() const;

    bool byPassWM() const;
    void setByPassWM(bool bypass);

    bool inEditMode() const;
    void setInEditMode(bool edit);

    bool isPreferredForShortcuts() const;
    void setIsPreferredForShortcuts(bool preferred);

    float maxLength() const;
    void setMaxLength(float length);

    int fontPixelSize() const;
    void setFontPixelSize(int size);

    int editThickness() const;
    void setEditThickness(int thickness);

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

    PlasmaQuick::ConfigView *configView();

    ViewPart::Effects *effects() const;
    ViewPart::Positioner *positioner() const;
    ViewPart::VisibilityManager *visibility() const;
    ViewPart::WindowsTracker *windowsTracker() const;

    Layout *managedLayout() const;
    void setManagedLayout(Layout *layout);

    KWayland::Client::PlasmaShellSurface *surface();

    void reconsiderScreen();

    //! these are signals that create crashes, such a example is the availableScreenRectChanged from corona
    //! when its containment is destroyed
    void disconnectSensitiveSignals();

public slots:
    Q_INVOKABLE void copyView();
    Q_INVOKABLE void removeView();

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

    Q_INVOKABLE bool isHighestPriorityView();

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
    void byPassWMChanged();
    void configWindowGeometryChanged(); // is called from config windows
    void contextMenuIsShownChanged();
    void dockLocationChanged();
    void editThicknessChanged();
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
    void positionerChanged();
    void screenGeometryChanged();
    void typeChanged();
    void visibilityChanged();
    void windowsTrackerChanged();
    void xChanged();
    void yChanged();

    void absGeometryChanged(const QRect &geometry);

private slots:
    void availableScreenRectChanged();
    void hideWindowsForSlidingOut();
    void preferredViewForShortcutsChangedSlot(Latte::View *view);
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
    bool m_byPassWM{true};
    bool m_containsMouse{false};
    bool m_inDelete{false};
    bool m_inEditMode{false};
    bool m_isPreferredForShortcuts{false};
    bool m_onPrimary{true};
    int m_fontPixelSize{ -1};
    int m_editThickness{24};
    int m_maxThickness{24};
    int m_normalThickness{24};
    int m_offset{0};
    float m_maxLength{1};

    Types::Alignment m_alignment{Types::Center};
    Types::ViewType m_type{Types::DockView};

    QRect m_localGeometry;
    QRect m_absGeometry;

    Layout *m_managedLayout{nullptr};
    QPointer<PlasmaQuick::ConfigView> m_configView;

    QPointer<ViewPart::ContextMenu> m_contextMenu;
    QPointer<ViewPart::Effects> m_effects;
    QPointer<ViewPart::Positioner> m_positioner;
    QPointer<ViewPart::VisibilityManager> m_visibility;
    QPointer<ViewPart::WindowsTracker> m_windowsTracker;

    //! Connections to release and bound for the managed layout
    std::array<QMetaObject::Connection, 5> connectionsManagedLayout;

    KWayland::Client::PlasmaShellSurface *m_shellSurface{nullptr};
};

}

#endif
