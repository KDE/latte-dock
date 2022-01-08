/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    void setWindowOnActivities(const WindowId &wid, const QStringList &activities) override;

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

    void setFrameExtents(QWindow *view, const QMargins &margins) override;
    void setInputMask(QWindow *window, const QRect &rect) override;

private:
    bool isAcceptableWindow(WindowId wid);
    bool isValidWindow(WindowId wid);

    QRect visibleGeometry(const WindowId &wid, const QRect &frameGeometry) const;

    void windowAddedProxy(WId wid);
    void windowChangedProxy(WId wid, NET::Properties prop1, NET::Properties2 prop2);

    QUrl windowUrl(WindowId wid);

    void checkShapeExtension();

private:
    //xcb_shape
    bool m_shapeExtensionChecked{false};
    bool m_shapeAvailable{false};
};

}
}

#endif // XWINDOWINTERFACE_H



