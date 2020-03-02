/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef POSITIONER_H
#define POSITIONER_H

//local
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

    Q_PROPERTY(bool inLocationAnimation READ inLocationAnimation NOTIFY inLocationAnimationChanged)
    Q_PROPERTY(bool inSlideAnimation READ inSlideAnimation WRITE setInSlideAnimation NOTIFY inSlideAnimationChanged)

    Q_PROPERTY(bool isStickedOnTopEdge READ isStickedOnTopEdge WRITE setIsStickedOnTopEdge NOTIFY isStickedOnTopEdgeChanged)
    Q_PROPERTY(bool isStickedOnBottomEdge READ isStickedOnBottomEdge WRITE setIsStickedOnBottomEdge NOTIFY isStickedOnBottomEdgeChanged)

    Q_PROPERTY(int currentScreenId READ currentScreenId NOTIFY currentScreenChanged)
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

    bool inLocationAnimation();

    bool inSlideAnimation() const;
    void setInSlideAnimation(bool active);

    bool isStickedOnTopEdge() const;
    void setIsStickedOnTopEdge(bool sticked);

    bool isStickedOnBottomEdge() const;
    void setIsStickedOnBottomEdge(bool sticked);

    void setScreenToFollow(QScreen *scr, bool updateScreenId = true);

    void reconsiderScreen();

    Latte::WindowSystem::WindowId trackedWindowId();

public slots:
    Q_INVOKABLE void hideDockDuringLocationChange(int goToLocation);
    Q_INVOKABLE void hideDockDuringMovingToLayout(QString layoutName);

    Q_INVOKABLE bool setCurrentScreen(const QString id);

    void syncGeometry();

signals:
    void currentScreenChanged();
    void edgeChanged();
    void screenGeometryChanged();
    void slideOffsetChanged();
    void windowSizeChanged();

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

    void onHideWindowsForSlidingOut();
    void inLocationAnimationChanged();
    void inSlideAnimationChanged();
    void isStickedOnTopEdgeChanged();
    void isStickedOnBottomEdgeChanged();

private slots:
    void screenChanged(QScreen *screen);
    void validateDockGeometry();
    void updateInLocationAnimation();
    void updateWaylandId();

private:
    void init();
    void initSignalingForLocationChangeSliding();
    void resizeWindow(QRect availableScreenRect = QRect());

    void updateFormFactor();
    void updatePosition(QRect availableScreenRect = QRect());

    void validateTopBottomBorders(QRect availableScreenRect, QRegion availableScreenRegion);

    QRect maximumNormalGeometry();

private:
    bool m_inDelete{false};
    bool m_inLocationAnimation{false};
    bool m_inSlideAnimation{false};

    bool m_isStickedOnTopEdge{false};
    bool m_isStickedOnBottomEdge{false};

    int m_slideOffset{0};

    //! it is used in order to enforce X11 to never miss window geometry
    QRect m_validGeometry;

    QPointer<Latte::View> m_view;
    QPointer<Latte::Corona> m_corona;

    QString m_screenToFollowId;
    QPointer<QScreen> m_screenToFollow;
    QTimer m_screenSyncTimer;

    QTimer m_validateGeometryTimer;

    //!used at sliding out/in animation
    QString m_moveToLayout;
    Plasma::Types::Location m_goToLocation{Plasma::Types::Floating};
    QScreen *m_goToScreen{nullptr};

    Latte::WindowSystem::WindowId m_trackedWindowId;
};

}
}

#endif
