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

#ifndef VISIBILITYMANAGER_H
#define VISIBILITYMANAGER_H

// local
#include "../plasma/quick/containmentview.h"
#include "../wm/abstractwindowinterface.h"
#include "../wm/windowinfowrap.h"
#include "../../liblatte2/types.h"

// Qt
#include <QObject>
#include <QTimer>

// Plasma
#include <Plasma/Containment>

namespace Latte {
class Corona;
class View;
namespace ViewPart {
class ScreenEdgeGhostWindow;
}
}

namespace Latte {
namespace ViewPart {

class VisibilityManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Latte::Types::Visibility mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(bool raiseOnDesktop READ raiseOnDesktop WRITE setRaiseOnDesktop NOTIFY raiseOnDesktopChanged)
    Q_PROPERTY(bool raiseOnActivity READ raiseOnActivity WRITE setRaiseOnActivity NOTIFY raiseOnActivityChanged)
    Q_PROPERTY(bool isHidden READ isHidden WRITE setIsHidden NOTIFY isHiddenChanged)
    Q_PROPERTY(bool blockHiding READ blockHiding WRITE setBlockHiding NOTIFY blockHidingChanged)
    Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)

    //! KWin Edges Support Options
    Q_PROPERTY(bool enableKWinEdges READ enableKWinEdges WRITE setEnableKWinEdges NOTIFY enableKWinEdgesChanged)
    Q_PROPERTY(bool supportsKWinEdges READ supportsKWinEdges NOTIFY supportsKWinEdgesChanged)

    Q_PROPERTY(int timerShow READ timerShow WRITE setTimerShow NOTIFY timerShowChanged)
    Q_PROPERTY(int timerHide READ timerHide WRITE setTimerHide NOTIFY timerHideChanged)

public:
    explicit VisibilityManager(PlasmaQuick::ContainmentView *view);
    virtual ~VisibilityManager();

    Latte::Types::Visibility mode() const;
    void setMode(Latte::Types::Visibility mode);

    void applyActivitiesToHiddenWindows(const QStringList &activities);

    bool raiseOnDesktop() const;
    void setRaiseOnDesktop(bool enable);

    bool raiseOnActivity() const;
    void setRaiseOnActivity(bool enable);

    bool isHidden() const;
    void setIsHidden(bool isHidden);

    bool blockHiding() const;
    void setBlockHiding(bool blockHiding);

    bool containsMouse() const;

    int timerShow() const;
    void setTimerShow(int msec);

    int timerHide() const;
    void setTimerHide(int msec);

    //! KWin Edges Support functions
    bool enableKWinEdges() const;
    void setEnableKWinEdges(bool enable);

    bool supportsKWinEdges() const;

    //! called for windowTracker to reset values
    void activeWindowDraggingStarted();

public slots:
    Q_INVOKABLE void hide();
    Q_INVOKABLE void show();

signals:
    void mustBeShown();
    void mustBeHide();

    void slideInFinished();

    void modeChanged();
    void raiseOnDesktopChanged();
    void raiseOnActivityChanged();
    void isHiddenChanged();
    void blockHidingChanged();
    void containsMouseChanged();
    void timerShowChanged();
    void timerHideChanged();

    //! KWin Edges Support signals
    void enableKWinEdgesChanged();
    void supportsKWinEdgesChanged();

private slots:
    void saveConfig();
    void restoreConfig();

private:
    void setContainsMouse(bool contains);

    void raiseView(bool raise);
    void raiseViewTemporarily();

    //! KWin Edges Support functions
    void createEdgeGhostWindow();
    void deleteEdgeGhostWindow();
    void updateKWinEdgesSupport();
    void updateGhostWindowState();

    void windowAdded(WindowId id);
    void updateStrutsBasedOnLayoutsAndActivities();
    void viewEventManager(QEvent *ev);

    QRect acceptableStruts();

private slots:
    void dodgeAllWindows();
    void dodgeActive();
    void dodgeMaximized();
    void updateHiddenState();

private:
    AbstractWindowInterface *m_wm;
    Types::Visibility m_mode{Types::None};
    std::array<QMetaObject::Connection, 5> m_connections;

    QTimer m_timerShow;
    QTimer m_timerHide;
    QTimer m_timerStartUp;

    bool m_isHidden{false};
    bool m_dragEnter{false};
    bool m_blockHiding{false};
    bool m_containsMouse{false};
    bool m_raiseTemporarily{false};
    bool m_raiseOnDesktopChange{false};
    bool m_raiseOnActivityChange{false};
    bool m_hideNow{false};

    QRect m_publishedStruts;

    //! KWin Edges
    bool m_enableKWinEdgesFromUser{true};
    std::array<QMetaObject::Connection, 1> m_connectionsKWinEdges;
    ScreenEdgeGhostWindow *m_edgeGhostWindow{nullptr};

    Latte::Corona *m_corona{nullptr};
    Latte::View *m_latteView{nullptr};

};

}
}
#endif // VISIBILITYMANAGER_H
