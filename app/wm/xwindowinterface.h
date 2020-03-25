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

#ifndef XWINDOWINTERFACE_H
#define XWINDOWINTERFACE_H

// local
#include <config-latte.h>
#include "abstractwindowinterface.h"
#include "windowinfowrap.h"

// Qt
#include <QObject>

// KDE
#include <KWindowInfo>
#include <KWindowEffects>


namespace Latte {
namespace WindowSystem {

class XWindowInterface : public AbstractWindowInterface
{
    Q_OBJECT

public:
    explicit XWindowInterface(QObject *parent = nullptr);
    ~XWindowInterface() override;

    void setViewExtraFlags(QObject *view, bool isPanelWindow = true, Latte::Types::Visibility mode = Latte::Types::WindowsGoBelow) override;
    void setViewStruts(QWindow &view, const QRect &rect, Plasma::Types::Location location) override;
    void setWindowOnActivities(QWindow &window, const QStringList &activities) override;

    void removeViewStruts(QWindow &view) override;

    WindowId activeWindow() override;
    WindowInfoWrap requestInfo(WindowId wid) override;
    WindowInfoWrap requestInfoActive() override;

    void skipTaskBar(const QDialog &dialog) override;
    void slideWindow(QWindow &view, Slide location) override;
    void enableBlurBehind(QWindow &view) override;

    void requestActivate(WindowId wid) override;
    void requestClose(WindowId wid) override;
    void requestMoveWindow(WindowId wid, QPoint from) override;
    void requestToggleIsOnAllDesktops(WindowId wid) override;
    void requestToggleKeepAbove(WindowId wid) override;
    void requestToggleMinimized(WindowId wid) override;
    void requestToggleMaximized(WindowId wid) override;
    void setKeepAbove(WindowId wid, bool active) override;
    void setKeepBelow(WindowId wid, bool active) override;

    bool windowCanBeDragged(WindowId wid) override;
    bool windowCanBeMaximized(WindowId wid) override;

    QIcon iconFor(WindowId wid) override;
    WindowId winIdFor(QString appId, QRect geometry) override;
    WindowId winIdFor(QString appId, QString title) override;
    AppData appDataFor(WindowId wid) override;

    void setActiveEdge(QWindow *view, bool active) override;

    void switchToNextVirtualDesktop() override;
    void switchToPreviousVirtualDesktop() override;

private:
    bool isAcceptableWindow(WindowId wid);
    bool isValidWindow(WindowId wid);

#if KF5_VERSION_MINOR >= 65
    QRect visibleGeometry(const WindowId &wid, const QRect &frameGeometry) const;
#endif

    void windowAddedProxy(WId wid);
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);

    QUrl windowUrl(WindowId wid);

};

}
}

#endif // XWINDOWINTERFACE_H



