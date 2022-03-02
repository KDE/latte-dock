/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLOATINGGAPWINDOW_H
#define FLOATINGGAPWINDOW_H

// local
#include "subwindow.h"
#include "../../lattecorona.h"
#include "../../wm/windowinfowrap.h"

// Qt
#include <QObject>
#include <QQuickView>
#include <QTimer>

namespace KWayland {
namespace Client {
class PlasmaShellSurface;
}
}

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {

//! What is the importance of this class?
//!
//! This window is responsible to identify if the mouse is still present
//! in the REAL FLOATING GAP between the VIEW and the SCREEN EDGE.
//! When VIEWS are REAL FLOATING then the VIEW Window is really placed
//! as it is shown to the user. In that case we need a way to check
//! where the mouse is even though it is OUTSIDE the VIEW or the
//! SCREENEDGEGHOSTWINDOW. The main functionality of FloatingGapWindow
//! is that it is TEMPORARILY shown/draw after a MUSTHIDE signal of
//! VisibilityManager was sent; in order to check if the mouse is still
//! inside the FLOATINGGAP. After it has really identified where mouse
//! is present, an FloatingGapWindow::asyncContainsMouse(contains) signal
//! is sent.

class FloatingGapWindow : public SubWindow
{
    Q_OBJECT

public:
    FloatingGapWindow(Latte::View *view);
    ~FloatingGapWindow() override;

    void callAsyncContainsMouse();

signals:
    void asyncContainsMouseChanged(bool contains); //called from visibility to check if mouse is in the free sensitive floating area

protected:
    bool event(QEvent *ev) override;
    QString validTitlePrefix() const override;
    void updateGeometry() override;

private:
    void triggerAsyncContainsMouseSignals();

private:
    bool m_containsMouse{false};

    bool m_inAsyncContainsMouse{false}; //called from visibility to check if mouse is in the free sensitive floating area

    QTimer m_asyncMouseTimer; //called from visibility to check if mouse is in the free sensitive floating area
};

}
}
#endif
