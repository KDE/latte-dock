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
class View;
}

namespace Latte {
namespace ViewPart {

class Positioner: public QObject
{
    Q_OBJECT

    Q_PROPERTY(int currentScreenId READ currentScreenId NOTIFY currentScreenChanged)
    Q_PROPERTY(QString currentScreenName READ currentScreenName NOTIFY currentScreenChanged)

public:
    Positioner(Latte::View *parent);
    virtual ~Positioner();

    int currentScreenId() const;
    QString currentScreenName() const;

    bool inLocationChangeAnimation();
    void setScreenToFollow(QScreen *scr, bool updateScreenId = true);

    void reconsiderScreen();

public slots:
    Q_INVOKABLE void hideDockDuringLocationChange(int goToLocation);
    Q_INVOKABLE void hideDockDuringMovingToLayout(QString layoutName);

    Q_INVOKABLE bool setCurrentScreen(const QString id);

    void syncGeometry();

signals:
    void currentScreenChanged();
    void edgeChanged();
    void screenGeometryChanged();
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
    void onHideWindowsForSlidingOut();
    void showDockAfterLocationChangeFinished();
    void showDockAfterScreenChangeFinished();
    void showDockAfterMovingToLayoutFinished();

private slots:
    void screenChanged(QScreen *screen);
    void validateDockGeometry();

private:
    void init();
    void initSignalingForLocationChangeSliding();
    void resizeWindow(QRect availableScreenRect = QRect());

    void updateFormFactor();
    void updatePosition(QRect availableScreenRect = QRect());

    QRect maximumNormalGeometry();

private:
    bool m_inDelete{false};

    //! it is used in order to enforce X11 to never miss window geometry
    QRect m_validGeometry;

    QPointer<Latte::View> m_view;

    QString m_screenToFollowId;
    QPointer<QScreen> m_screenToFollow;
    QTimer m_screenSyncTimer;

    QTimer m_validateGeometryTimer;

    //!used at sliding out/in animation
    QString m_moveToLayout;
    Plasma::Types::Location m_goToLocation{Plasma::Types::Floating};
    QScreen *m_goToScreen{nullptr};
};

}
}

#endif
