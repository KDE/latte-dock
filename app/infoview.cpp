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

#include "infoview.h"
#include "panelshadows_p.h"
#include "abstractwindowinterface.h"

#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>

#include <Plasma/Package>

#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

namespace Latte {

InfoView::InfoView(DockCorona *corona, QString message, QScreen *screen, QWindow *parent)
    : QQuickView(parent),
      m_corona(corona),
      m_message(message),
      m_screen(screen)
{
    setupWaylandIntegration();

    setResizeMode(QQuickView::SizeViewToRootObject);
    setColor(QColor(Qt::transparent));
    setIcon(qGuiApp->windowIcon());

    setScreen(screen);
    setFlags(wFlags());
    syncGeometry();

    init();
}

InfoView::~InfoView()
{
    qDebug() << "InfoView deleting ...";

    if (m_shellSurface) {
        delete m_shellSurface;
        m_shellSurface = nullptr;
    }

}

void InfoView::init()
{
    setDefaultAlphaBuffer(true);
    setColor(Qt::transparent);
    PanelShadows::self()->addWindow(this);

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
    kdeclarative.setupBindings();
    auto source = QUrl::fromLocalFile(m_corona->kPackage().filePath("infoviewui"));
    setSource(source);
    rootObject()->setProperty("message", m_message);

    syncGeometry();
}

inline Qt::WindowFlags InfoView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

void InfoView::syncGeometry()
{
    /*if (!m_dockView->containment() || !rootObject())
        return;

    const auto location = m_dockView->containment()->location();
    const auto sGeometry = screen()->geometry();

    int clearThickness = m_dockView->normalThickness();

    QPoint position{0, 0};

    switch (m_dockView->containment()->formFactor()) {
        case Plasma::Types::Horizontal: {
            const QSize size(rootObject()->width(), rootObject()->height());
            setMaximumSize(size);
            setMinimumSize(size);
            resize(size);

            if (location == Plasma::Types::TopEdge) {
                position = {sGeometry.center().x() - size.width() / 2
                            , sGeometry.y() + clearThickness
                           };
            } else if (location == Plasma::Types::BottomEdge) {
                position = {sGeometry.center().x() - size.width() / 2
                            , sGeometry.y() + sGeometry.height() - clearThickness - size.height()
                           };
            }
        }
        break;

        case Plasma::Types::Vertical: {
            const QSize size(rootObject()->width(), rootObject()->height());
            setMaximumSize(size);
            setMinimumSize(size);
            resize(size);

            if (location == Plasma::Types::LeftEdge) {
                position = {sGeometry.x() + clearThickness
                            , sGeometry.center().y() - size.height() / 2
                           };
            } else if (location == Plasma::Types::RightEdge) {
                position = {sGeometry.x() + sGeometry.width() - clearThickness - size.width()
                            , sGeometry.center().y() - size.height() / 2
                           };
            }
        }
        break;

        default:
            qWarning() << "no sync geometry, wrong formFactor";
            break;
    }

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }*/
}

void InfoView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    WindowSystem::self().setDockExtraFlags(*this);
    setFlags(wFlags());

    WindowSystem::self().enableBlurBehind(*this);

    syncGeometry();

    QTimer::singleShot(400, this, &InfoView::syncGeometry);
}

void InfoView::setupWaylandIntegration()
{
    if (m_shellSurface) {
        // already setup
        return;
    }

    if (m_corona) {
        using namespace KWayland::Client;
        PlasmaShell *interface = m_corona->waylandDockCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland dock window surface was created...";

        m_shellSurface = interface->createSurface(s, this);

        syncGeometry();
    }
}

bool InfoView::event(QEvent *e)
{
    if (e->type() == QEvent::PlatformSurface) {
        if (auto pe = dynamic_cast<QPlatformSurfaceEvent *>(e)) {
            switch (pe->surfaceEventType()) {
                case QPlatformSurfaceEvent::SurfaceCreated:

                    if (m_shellSurface) {
                        break;
                    }

                    setupWaylandIntegration();
                    break;

                case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
                    if (m_shellSurface) {
                        delete m_shellSurface;
                        m_shellSurface = nullptr;
                    }

                    PanelShadows::self()->removeWindow(this);
                    break;
            }
        }
    }

    return InfoView::event(e);
}

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
