/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWWINDOWSTRACKER_H
#define VIEWWINDOWSTRACKER_H

// local
#include "../../wm/abstractwindowinterface.h"

// Qt
#include <QObject>

namespace Latte{
class View;

namespace ViewPart {
namespace TrackerPart {
class AllScreensTracker;
class CurrentScreenTracker;
}
}

namespace WindowSystem {
class AbstractWindowInterface;
}
}

namespace Latte {
namespace ViewPart {

class WindowsTracker : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

    Q_PROPERTY(Latte::ViewPart::TrackerPart::CurrentScreenTracker *currentScreen READ currentScreen NOTIFY currentScreenChanged)
    Q_PROPERTY(Latte::ViewPart::TrackerPart::AllScreensTracker *allScreens READ allScreens NOTIFY allScreensChanged)

public:
    explicit WindowsTracker(Latte::View *parent);
    virtual ~WindowsTracker();

    bool enabled() const;
    void setEnabled(bool active);

    TrackerPart::AllScreensTracker *allScreens() const;
    TrackerPart::CurrentScreenTracker *currentScreen() const;

    Latte::View *view() const;
    WindowSystem::AbstractWindowInterface *wm() const;

public slots:
    Q_INVOKABLE void switchToNextActivity();
    Q_INVOKABLE void switchToPreviousActivity();
    Q_INVOKABLE void switchToNextVirtualDesktop();
    Q_INVOKABLE void switchToPreviousVirtualDesktop();

signals:
    void enabledChanged();
    void allScreensChanged();
    void currentScreenChanged();

private:
    Latte::View *m_latteView{nullptr};
    WindowSystem::AbstractWindowInterface *m_wm{nullptr};

    TrackerPart::AllScreensTracker *m_allScreensTracker{nullptr};
    TrackerPart::CurrentScreenTracker *m_currentScreenTracker{nullptr};
};

}
}

#endif
