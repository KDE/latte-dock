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

#include "abstractwindowinterface.h"
#include "windowinfowrap.h"

#include <QObject>

#include <KWindowInfo>
#include <KWindowEffects>

namespace Latte {

class XWindowInterface : public AbstractWindowInterface {
    Q_OBJECT

public:
    explicit XWindowInterface(QObject *parent = nullptr);
    ~XWindowInterface() override;

    void setDockExtraFlags(QQuickWindow &view) override;
    void setDockStruts(WindowId dockId, const QRect &dockRect
                       , const QScreen &screen, Plasma::Types::Location location) const override;

    void removeDockStruts(WindowId dockId) const override;

    WindowId activeWindow() const override;
    WindowInfoWrap requestInfo(WindowId wid) const override;
    WindowInfoWrap requestInfoActive() const override;
    bool isOnCurrentDesktop(WindowId wid) const override;
    const std::list<WindowId> &windows() const override;

    void skipTaskBar(const QDialog &dialog) const override;
    void slideWindow(QQuickWindow &view, Slide location) const override;
    void enableBlurBehind(QQuickWindow &view) const override;

private:
    bool isValidWindow(const KWindowInfo &winfo) const;
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);

    WindowId m_desktopId;
};

}

#endif // XWINDOWINTERFACE_H



