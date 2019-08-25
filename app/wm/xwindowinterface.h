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

    void setViewExtraFlags(QWindow &view) override;
    void setViewStruts(QWindow &view, const QRect &rect
                       , Plasma::Types::Location location) override;
    void setWindowOnActivities(QWindow &window, const QStringList &activities) override;

    void removeViewStruts(QWindow &view) const override;

    WindowId activeWindow() const override;
    WindowInfoWrap requestInfo(WindowId wid) const override;
    WindowInfoWrap requestInfoActive() const override;

    void setKeepAbove(const QDialog &dialog, bool above = true) const override;
    void skipTaskBar(const QDialog &dialog) const override;
    void slideWindow(QWindow &view, Slide location) const override;
    void enableBlurBehind(QWindow &view) const override;

    void requestActivate(WindowId wid) const override;
    void requestClose(WindowId wid) const override;
    void requestMoveWindow(WindowId wid, QPoint from) const override;
    void requestToggleIsOnAllDesktops(WindowId wid) const override;
    void requestToggleKeepAbove(WindowId wid) const override;
    void requestToggleMinimized(WindowId wid) const override;
    void requestToggleMaximized(WindowId wid) const override;

    bool windowCanBeDragged(WindowId wid) const override;
    bool windowCanBeMaximized(WindowId wid) const override;

    QIcon iconFor(WindowId wid) const override;
    WindowId winIdFor(QString appId, QRect geometry) const override;   
    AppData appDataFor(WindowId wid) const override;

    void setEdgeStateFor(QWindow *view, bool active) const override;

    void switchToNextVirtualDesktop() const override;
    void switchToPreviousVirtualDesktop() const override;

private:
    bool hasScreenGeometry(const KWindowInfo &winfo) const;
    bool isValidWindow(WindowId wid) const;
    bool isValidWindow(const KWindowInfo &winfo) const;
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);

    QUrl windowUrl(WindowId wid) const;

private:
    WindowId m_desktopId{-1};
};

}
}

#endif // XWINDOWINTERFACE_H



