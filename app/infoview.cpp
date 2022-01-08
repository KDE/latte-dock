/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "infoview.h"

// local
#include <config-latte.h>
#include "wm/abstractwindowinterface.h"
#include "view/panelshadows_p.h"

// Qt
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QScreen>

// KDE
#include <KLocalizedContext>
#include <KDeclarative/KDeclarative>
#include <KWindowSystem>
#include <KWayland/Client/plasmashell.h>
#include <KWayland/Client/surface.h>

// Plasma
#include <Plasma/Package>

namespace Latte {

InfoView::InfoView(Latte::Corona *corona, QString message, QScreen *screen, QWindow *parent)
    : QQuickView(parent),
      m_corona(corona),
      m_message(message),
      m_screen(screen)
{
    m_id = QString::number(qrand() % 1000);

    setTitle(validTitle());

    setupWaylandIntegration();

    setResizeMode(QQuickView::SizeViewToRootObject);
    setColor(QColor(Qt::transparent));
    setDefaultAlphaBuffer(true);

    setIcon(qGuiApp->windowIcon());

    setScreen(screen);
    setFlags(wFlags());

    if (KWindowSystem::isPlatformX11()) {
        m_trackedWindowId = winId();
        m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);
    } else {
        connect(m_corona->wm(), &WindowSystem::AbstractWindowInterface::latteWindowAdded, this, &InfoView::updateWaylandId);
    }

    init();
}

InfoView::~InfoView()
{
    PanelShadows::self()->removeWindow(this);

    qDebug() << "InfoView deleting ...";

    if (m_shellSurface) {
        delete m_shellSurface;
        m_shellSurface = nullptr;
    }
}

void InfoView::init()
{
    rootContext()->setContextProperty(QStringLiteral("infoWindow"), this);

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine());
    kdeclarative.setTranslationDomain(QStringLiteral("latte-dock"));
    kdeclarative.setupContext();
    kdeclarative.setupEngine(engine());

    auto source = QUrl::fromLocalFile(m_corona->kPackage().filePath("infoviewui"));
    setSource(source);

    rootObject()->setProperty("message", m_message);

    syncGeometry();
}

QString InfoView::validTitle() const
{
    return "#layoutinfowindow#" + m_id;
}

Plasma::FrameSvg::EnabledBorders InfoView::enabledBorders() const
{
    return m_borders;
}


inline Qt::WindowFlags InfoView::wFlags() const
{
    return (flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) & ~Qt::WindowDoesNotAcceptFocus;
}

void InfoView::syncGeometry()
{
    const QSize size(rootObject()->width(), rootObject()->height());
    const auto sGeometry = screen()->geometry();

    setMaximumSize(size);
    setMinimumSize(size);
    resize(size);

    QPoint position{sGeometry.center().x() - size.width() / 2, sGeometry.center().y() - size.height() / 2 };

    setPosition(position);

    if (m_shellSurface) {
        m_shellSurface->setPosition(position);
    }
}

void InfoView::showEvent(QShowEvent *ev)
{
    QQuickWindow::showEvent(ev);

    m_corona->wm()->setViewExtraFlags(this);
    setFlags(wFlags());

    m_corona->wm()->enableBlurBehind(*this);

    syncGeometry();

    QTimer::singleShot(400, this, &InfoView::syncGeometry);

    PanelShadows::self()->addWindow(this);
    PanelShadows::self()->setEnabledBorders(this, m_borders);
}

void InfoView::updateWaylandId()
{
    Latte::WindowSystem::WindowId newId = m_corona->wm()->winIdFor("latte-dock", validTitle());

    if (m_trackedWindowId != newId) {
        if (!m_trackedWindowId.isNull()) {
            m_corona->wm()->unregisterIgnoredWindow(m_trackedWindowId);
        }

        m_trackedWindowId = newId;
        m_corona->wm()->registerIgnoredWindow(m_trackedWindowId);
    }
}

void InfoView::setupWaylandIntegration()
{
    if (m_shellSurface) {
        // already setup
        return;
    }

    if (m_corona) {
        using namespace KWayland::Client;
        PlasmaShell *interface = m_corona->waylandCoronaInterface();

        if (!interface) {
            return;
        }

        Surface *s = Surface::fromWindow(this);

        if (!s) {
            return;
        }

        qDebug() << "wayland dock window surface was created...";

        m_shellSurface = interface->createSurface(s, this);
        m_corona->wm()->setViewExtraFlags(m_shellSurface);
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

    return QQuickWindow::event(e);
}

void InfoView::setOnActivities(QStringList activities)
{
    KWindowSystem::setOnActivities(winId(), activities);
}

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
