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

#ifndef ABSTRACTWINDOWINTERFACE_H
#define ABSTRACTWINDOWINTERFACE_H

// local
#include "schemecolors.h"
#include "tasktools.h"
#include "windowinfowrap.h"
#include "tracker/windowstracker.h"
#include "../liblatte2/types.h"
#include "../liblatte2/extras.h"

// C++
#include <unordered_map>
#include <list>

// Qt
#include <QObject>
#include <QWindow>
#include <QDialog>
#include <QMap>
#include <QRect>
#include <QPoint>
#include <QPointer>
#include <QScreen>
#include <QTimer>

// KDE
#include <KSharedConfig>
#include <KActivities/Consumer>

// Plasma
#include <Plasma>

namespace Latte {
class Corona;
namespace WindowSystem {
namespace Tracker {
class Schemes;
class Windows;
}
}
}

namespace Latte {
namespace WindowSystem {

class AbstractWindowInterface : public QObject
{
    Q_OBJECT

public:
    enum class Slide
    {
        None,
        Top,
        Left,
        Bottom,
        Right,
    };

    explicit AbstractWindowInterface(QObject *parent = nullptr);
    virtual ~AbstractWindowInterface();

    virtual void setViewExtraFlags(QWindow &view) = 0;
    virtual void setViewStruts(QWindow &view, const QRect &rect
                               , Plasma::Types::Location location) = 0;
    virtual void setWindowOnActivities(QWindow &window, const QStringList &activities) = 0;

    virtual void removeViewStruts(QWindow &view) const = 0;

    virtual WindowId activeWindow() const = 0;
    virtual WindowInfoWrap requestInfo(WindowId wid) const = 0;
    virtual WindowInfoWrap requestInfoActive() const = 0;

    virtual void setKeepAbove(const QDialog &dialog, bool above = true) const = 0;
    virtual void skipTaskBar(const QDialog &dialog) const = 0;
    virtual void slideWindow(QWindow &view, Slide location) const = 0;
    virtual void enableBlurBehind(QWindow &view) const = 0;
    virtual void setActiveEdge(QWindow *view, bool active) const = 0;

    virtual void requestActivate(WindowId wid) const = 0;
    virtual void requestClose(WindowId wid) const = 0;
    virtual void requestMoveWindow(WindowId wid, QPoint from) const = 0;
    virtual void requestToggleIsOnAllDesktops(WindowId wid) const = 0;
    virtual void requestToggleKeepAbove(WindowId wid) const = 0;
    virtual void requestToggleMinimized(WindowId wid) const = 0;
    virtual void requestToggleMaximized(WindowId wid) const = 0;

    virtual bool windowCanBeDragged(WindowId wid) const = 0;
    virtual bool windowCanBeMaximized(WindowId wid) const = 0;

    virtual QIcon iconFor(WindowId wid) const = 0;
    virtual WindowId winIdFor(QString appId, QRect geometry) const = 0;
    virtual AppData appDataFor(WindowId wid) const = 0;

    bool inCurrentDesktopActivity(const WindowInfoWrap &winfo) const;

    bool isIgnored(const WindowId &wid);
    bool isRegisteredPlasmaPanel(const WindowId &wid);

    QString currentDesktop() const;
    QString currentActivity() const;

    virtual void registerIgnoredWindow(WindowId wid);
    virtual void unregisterIgnoredWindow(WindowId wid);

    void registerPlasmaPanel(WindowId wid);
    void unregisterPlasmaPanel(WindowId wid);

    void switchToNextActivity();
    void switchToPreviousActivity();

    virtual void switchToNextVirtualDesktop() const = 0;
    virtual void switchToPreviousVirtualDesktop() const = 0;

    Latte::Corona *corona();
    Tracker::Schemes *schemesTracker();
    Tracker::Windows *windowsTracker() const;

signals:
    void activeWindowChanged(WindowId wid);
    void windowChanged(WindowId winfo);
    void windowAdded(WindowId wid);
    void windowRemoved(WindowId wid);
    void currentDesktopChanged();
    void currentActivityChanged();

    void latteWindowAdded();

protected:
    QString m_currentDesktop;
    QString m_currentActivity;

    //! windows that must be ignored from tracking, a good example are Latte::Views and
    //! their Configuration windows
    QList<WindowId> m_ignoredWindows;
    //! identified plasma panels
    QList<WindowId> m_plasmaPanels;

    QPointer<KActivities::Consumer> m_activities;

    //! Sending too fast plenty of signals for the same window
    //! has no reason and can create HIGH CPU usage. This Timer
    //! can delay the batch sending of signals for the same window
    WindowId m_windowChangedWaiting;
    QTimer m_windowWaitingTimer;

    //! Plasma taskmanager rules ile
    KSharedConfig::Ptr rulesConfig;

    void considerWindowChanged(WindowId wid);

    bool isPlasmaDesktop(const QRect &wGeometry) const;
    bool isPlasmaPanel(const QRect &wGeometry) const;

private slots:
    void windowRemovedSlot(WindowId wid);

private:
    Latte::Corona *m_corona;
    Tracker::Schemes *m_schemesTracker;
    Tracker::Windows *m_windowsTracker;
};

}
}

#endif // ABSTRACTWINDOWINTERFACE_H
