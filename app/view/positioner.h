/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef POSITIONER_H
#define POSITIONER_H

//local
#include <coretypes.h>
#include "../wm/abstractwindowinterface.h"
#include "../wm/windowinfowrap.h"

// Qt
#include <QObject>
#include <QPointer>
#include <QScreen>
#include <QTimer>

// Plasma
#include <Plasma/Containment>

namespace Plasma {
class Types;
}

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {

class Positioner: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool inRelocationAnimation READ inRelocationAnimation NOTIFY inRelocationAnimationChanged)
    Q_PROPERTY(bool inRelocationShowing READ inRelocationShowing WRITE setInRelocationShowing NOTIFY inRelocationShowingChanged)
    Q_PROPERTY(bool inSlideAnimation READ inSlideAnimation WRITE setInSlideAnimation NOTIFY inSlideAnimationChanged)

    Q_PROPERTY(bool isOffScreen READ isOffScreen NOTIFY isOffScreenChanged)
    Q_PROPERTY(bool isStickedOnTopEdge READ isStickedOnTopEdge WRITE setIsStickedOnTopEdge NOTIFY isStickedOnTopEdgeChanged)
    Q_PROPERTY(bool isStickedOnBottomEdge READ isStickedOnBottomEdge WRITE setIsStickedOnBottomEdge NOTIFY isStickedOnBottomEdgeChanged)

    Q_PROPERTY(int currentScreenId READ currentScreenId NOTIFY currentScreenChanged)

    Q_PROPERTY(QRect canvasGeometry READ canvasGeometry NOTIFY canvasGeometryChanged)

    //! animating window slide
    Q_PROPERTY(int slideOffset READ slideOffset WRITE setSlideOffset NOTIFY slideOffsetChanged)
    Q_PROPERTY(QString currentScreenName READ currentScreenName NOTIFY currentScreenChanged)

public:
    Positioner(Latte::View *parent);
    virtual ~Positioner();

    int currentScreenId() const;
    QString currentScreenName() const;

    int slideOffset() const;
    void setSlideOffset(int offset);

    bool inLayoutUnloading();
    bool inRelocationAnimation();

    bool inRelocationShowing() const;
    void setInRelocationShowing(bool active);

    bool inSlideAnimation() const;
    void setInSlideAnimation(bool active);

    bool isCursorInsideView() const;

    bool isStickedOnTopEdge() const;
    void setIsStickedOnTopEdge(bool sticked);

    bool isStickedOnBottomEdge() const;
    void setIsStickedOnBottomEdge(bool sticked);

    bool isOffScreen() const;

    QRect canvasGeometry();

    void setScreenToFollow(QScreen *scr, bool updateScreenId = true);
    void setWindowOnActivities(const Latte::WindowSystem::WindowId &wid, const QStringList &activities);

    void reconsiderScreen();

    Latte::WindowSystem::WindowId trackedWindowId();

public slots:
    Q_INVOKABLE void setNextLocation(const QString layoutName, const int screensGroup, QString screenName, int edge, int alignment);
    Q_INVOKABLE void slideInDuringStartup();

    void syncGeometry();

    //! direct geometry calculations without any protections or checks
    //! that might prevent them. It must be called with care.
    void immediateSyncGeometry();

    void slideOutDuringExit(Plasma::Types::Location location = Plasma::Types::Floating);

    void initDelayedSignals();
    void updateWaylandId();

signals:
    void canvasGeometryChanged();
    void currentScreenChanged();
    void edgeChanged();
    void screenGeometryChanged();
    void slideOffsetChanged();
    void windowSizeChanged();
    void winIdChanged();

    //! these two signals are used from config ui and containment ui
    //! in order to orchestrate an animated hiding/showing of dock
    //! during changing location
    void hidingForRelocationStarted();
    void hidingForRelocationFinished();
    void showingAfterRelocationFinished();

    void startupFinished(); //called from containment qml end of startup sequence

    void onHideWindowsForSlidingOut();
    void inRelocationAnimationChanged();
    void inRelocationShowingChanged();
    void inSlideAnimationChanged();
    void isOffScreenChanged();
    void isStickedOnTopEdgeChanged();
    void isStickedOnBottomEdgeChanged();

private slots:
    void onScreenChanged(QScreen *screen);
    void onCurrentLayoutIsSwitching(const QString &layoutName);
    void onLastRepositionApplyEvent();
    void onStartupFinished();

    void validateDockGeometry();
    void updateInRelocationAnimation();
    void syncLatteViews();
    void updateContainmentScreen();

private:
    void init();
    void initSignalingForLocationChangeSliding();

    void updateFormFactor();
    void resizeWindow(QRect availableScreenRect = QRect());
    void updatePosition(QRect availableScreenRect = QRect());
    void updateCanvasGeometry(QRect availableScreenRect = QRect());

    void validateTopBottomBorders(QRect availableScreenRect, QRegion availableScreenRegion);

    void setCanvasGeometry(const QRect &geometry);

    bool isLastHidingRelocationEvent() const;

    QRect maximumNormalGeometry(QRect screenGeometry = QRect());

    WindowSystem::AbstractWindowInterface::Slide slideLocation(Plasma::Types::Location location);

private:
    bool m_inDelete{false};
    bool m_inLayoutUnloading{false};
    bool m_inRelocationAnimation{false};
    bool m_inRelocationShowing{false};
    bool m_inSlideAnimation{false};
    bool m_inStartup{true};

    bool m_isStickedOnTopEdge{false};
    bool m_isStickedOnBottomEdge{false};

    int m_slideOffset{0};

    QRect m_canvasGeometry;
    //! it is used in order to enforce X11 to never miss window geometry
    QRect m_validGeometry;
    //! it is used to update geometry calculations without requesting no needed Corona calculations
    QRect m_lastAvailableScreenRect;
    QRegion m_lastAvailableScreenRegion;

    QPointer<Latte::View> m_view;
    QPointer<Latte::Corona> m_corona;

    QString m_screenNameToFollow;
    QPointer<QScreen> m_screenToFollow;
    QTimer m_screenSyncTimer;
    QTimer m_syncGeometryTimer;
    QTimer m_validateGeometryTimer;

    //!used for relocation properties group
    bool m_repositionFromViewSettingsWindow{false};
    bool m_repositionIsAnimated{false};

    QString m_nextLayoutName;
    Latte::Types::ScreensGroup m_nextScreensGroup{Latte::Types::SingleScreenGroup};
    QString m_nextScreenName;
    QScreen *m_nextScreen{nullptr};
    Plasma::Types::Location m_nextScreenEdge{Plasma::Types::Floating};
    Latte::Types::Alignment m_nextAlignment{Latte::Types::NoneAlignment};

    Latte::WindowSystem::WindowId m_trackedWindowId;
};

}
}

#endif
