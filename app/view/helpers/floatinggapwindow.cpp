/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "floatinggapwindow.h"

// local
#include "../view.h"

// Qt
#include <QDebug>
#include <QSurfaceFormat>
#include <QQuickView>
#include <QTimer>

// KDE
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>
#include <KWindowSystem>

// X11
#include <NETWM>

namespace Latte {
namespace ViewPart {

FloatingGapWindow::FloatingGapWindow(Latte::View *view) :
    SubWindow(view, QString("Floating Gap Window"))
{
    if (m_debugMode) {
        m_showColor = QColor("green");
        m_hideColor = QColor("red");
    } else {
        m_showColor = QColor(Qt::transparent);
        m_hideColor = QColor(Qt::transparent);

        m_showColor.setAlpha(0);
        m_hideColor.setAlpha(1);
    }

    setColor(m_showColor);

    //! this timer is used in order to identify if mouse is still present in sensitive floating
    //! areas and in such case to prevent a real-floating view to hide itself
    m_asyncMouseTimer.setSingleShot(true);
    m_asyncMouseTimer.setInterval(200);
    connect(&m_asyncMouseTimer, &QTimer::timeout, this, [this]() {
        if (m_inAsyncContainsMouse && !m_containsMouse) {
            emit asyncContainsMouseChanged(false);
            hideWithMask();
            m_inAsyncContainsMouse = false;
        }
    });


    updateGeometry();
    hideWithMask();
}

FloatingGapWindow::~FloatingGapWindow()
{
}

QString FloatingGapWindow::validTitlePrefix() const
{
    return QString("#subfloatgap#");
}

void FloatingGapWindow::updateGeometry()
{
    if (m_latteView->positioner()->slideOffset() != 0) {
        return;
    }

    QRect newGeometry;

    m_thickness = m_latteView->screenEdgeMargin();

    int length = m_latteView->formFactor() == Plasma::Types::Horizontal ? m_latteView->absoluteGeometry().width() : m_latteView->absoluteGeometry().height();

    if (m_latteView->location() == Plasma::Types::BottomEdge) {
        int xF = qMax(m_latteView->screenGeometry().left(), m_latteView->absoluteGeometry().left());
        newGeometry.moveLeft(xF);
        newGeometry.moveTop(m_latteView->screenGeometry().bottom() - m_thickness);
    } else if (m_latteView->location() == Plasma::Types::TopEdge) {
        int xF = qMax(m_latteView->screenGeometry().left(), m_latteView->absoluteGeometry().left());
        newGeometry.moveLeft(xF);
        newGeometry.moveTop(m_latteView->screenGeometry().top());
    } else if (m_latteView->location() == Plasma::Types::LeftEdge) {
        int yF = qMax(m_latteView->screenGeometry().top(), m_latteView->absoluteGeometry().top());
        newGeometry.moveLeft(m_latteView->screenGeometry().left());
        newGeometry.moveTop(yF);
    } else if (m_latteView->location() == Plasma::Types::RightEdge) {
        int yF = qMax(m_latteView->screenGeometry().top(), m_latteView->absoluteGeometry().top());
        newGeometry.moveLeft(m_latteView->screenGeometry().right() - m_thickness);
        newGeometry.moveTop(yF);
    }

    if (m_latteView->formFactor() == Plasma::Types::Horizontal) {
        newGeometry.setWidth(length);
        newGeometry.setHeight(m_thickness + 1);
    } else {
        newGeometry.setWidth(m_thickness + 1);
        newGeometry.setHeight(length);
    }

    m_calculatedGeometry = newGeometry;

    emit calculatedGeometryChanged();
}

bool FloatingGapWindow::event(QEvent *e)
{
    if (e->type() == QEvent::DragEnter || e->type() == QEvent::DragMove) {
        m_containsMouse = true;

    } else if (e->type() == QEvent::Enter) {
        m_containsMouse = true;

        triggerAsyncContainsMouseSignals();
    } else if (e->type() == QEvent::Leave || e->type() == QEvent::DragLeave) {
        m_containsMouse = false;

        if (m_inAsyncContainsMouse) {
            m_asyncMouseTimer.stop();
            m_inAsyncContainsMouse = false;
            emit asyncContainsMouseChanged(true);
        }
    }

    return SubWindow::event(e);
}

void FloatingGapWindow::callAsyncContainsMouse()
{
    m_inAsyncContainsMouse = true;
    m_asyncMouseTimer.start();
    showWithMask();
}

void FloatingGapWindow::triggerAsyncContainsMouseSignals()
{
    if (!m_inAsyncContainsMouse) {
        return;
    }

    //! this function is called QEvent::Enter
    m_asyncMouseTimer.stop();
    hideWithMask();
}

}
}
