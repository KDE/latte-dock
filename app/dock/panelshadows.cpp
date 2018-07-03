/*
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "panelshadows_p.h"

#include <QWindow>
#include <QPainter>

#include <config-latte.h>

#include <KWindowSystem>
#if HAVE_X11
    #include <QX11Info>
    #include <X11/Xatom.h>
    #include <X11/Xlib.h>
    #include <X11/Xlib-xcb.h>
    #include <fixx11h.h>
#endif

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/shadow.h>
#include <KWayland/Client/shm_pool.h>
#include <KWayland/Client/surface.h>

#include <qdebug.h>

class PanelShadows::Private
{
public:
    Private(PanelShadows *shadows)
        : q(shadows)
#if HAVE_X11
        , _connection(0x0),
          _gc(0x0)
        , m_isX11(KWindowSystem::isPlatformX11())
#endif
    {
        setupWaylandIntegration();
    }

    ~Private() {
        // Do not call clearPixmaps() from here: it creates new QPixmap(),
        // which causes a crash when application is stopping.
        freeX11Pixmaps();
    }

    void freeX11Pixmaps();
    void freeWaylandBuffers();
    void clearPixmaps();
    void setupPixmaps();
    Qt::HANDLE createPixmap(const QPixmap &source);
    void initPixmap(const QString &element);
    QPixmap initEmptyPixmap(const QSize &size);
    void updateShadow(const QWindow *window, Plasma::FrameSvg::EnabledBorders);
    void updateShadowX11(const QWindow *window, Plasma::FrameSvg::EnabledBorders);
    void updateShadowWayland(const QWindow *window, Plasma::FrameSvg::EnabledBorders);
    void clearShadow(const QWindow *window);
    void clearShadowX11(const QWindow *window);
    void clearShadowWayland(const QWindow *window);
    void updateShadows();
    void windowDestroyed(QObject *deletedObject);
    void setupData(Plasma::FrameSvg::EnabledBorders enabledBorders);

    void setupWaylandIntegration();

    PanelShadows *q;
    QList<QPixmap> m_shadowPixmaps;

    QPixmap m_emptyCornerPix;
    QPixmap m_emptyCornerLeftPix;
    QPixmap m_emptyCornerTopPix;
    QPixmap m_emptyCornerRightPix;
    QPixmap m_emptyCornerBottomPix;
    QPixmap m_emptyVerticalPix;
    QPixmap m_emptyHorizontalPix;

#if HAVE_X11
    //! xcb connection
    xcb_connection_t *_connection;

    //! graphical context
    xcb_gcontext_t _gc;
    bool m_isX11;
#endif

    struct Wayland
    {
        KWayland::Client::ShadowManager *manager = nullptr;
        KWayland::Client::ShmPool *shmPool = nullptr;

        QList<KWayland::Client::Buffer::Ptr> shadowBuffers;
    };
    Wayland m_wayland;

    QHash<Plasma::FrameSvg::EnabledBorders, QVector<unsigned long>> data;
    QHash<const QWindow *, Plasma::FrameSvg::EnabledBorders> m_windows;
};

class PanelShadowsSingleton
{
public:
    PanelShadowsSingleton() {
    }

    PanelShadows self;
};

Q_GLOBAL_STATIC(PanelShadowsSingleton, privatePanelShadowsSelf)

PanelShadows::PanelShadows(QObject *parent, const QString &prefix)
    : Plasma::Svg(parent),
      d(new Private(this))
{
    setImagePath(prefix);
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateShadows()));
}

PanelShadows::~PanelShadows()
{
    delete d;
}

PanelShadows *PanelShadows::self()
{
    return &privatePanelShadowsSelf->self;
}

void PanelShadows::addWindow(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
    if (!window) {
        return;
    }

    d->m_windows[window] = enabledBorders;
    d->updateShadow(window, enabledBorders);
    connect(window, SIGNAL(destroyed(QObject *)),
            this, SLOT(windowDestroyed(QObject *)), Qt::UniqueConnection);
}

void PanelShadows::removeWindow(const QWindow *window)
{
    if (!d->m_windows.contains(window)) {
        return;
    }

    d->m_windows.remove(window);
    disconnect(window, 0, this, 0);
    d->clearShadow(window);

    if (d->m_windows.isEmpty()) {
        d->clearPixmaps();
    }
}

void PanelShadows::setEnabledBorders(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
    if (!window || !d->m_windows.contains(window)) {
        return;
    }

    d->m_windows[window] = enabledBorders;
    d->updateShadow(window, enabledBorders);
}


void PanelShadows::Private::windowDestroyed(QObject *deletedObject)
{
    m_windows.remove(static_cast<QWindow *>(deletedObject));

    if (m_windows.isEmpty()) {
        clearPixmaps();
    }
}

void PanelShadows::Private::updateShadows()
{
    setupPixmaps();
    QHash<const QWindow *, Plasma::FrameSvg::EnabledBorders>::const_iterator i;

    for (i = m_windows.constBegin(); i != m_windows.constEnd(); ++i) {
        updateShadow(i.key(), i.value());
    }
}

Qt::HANDLE PanelShadows::Private::createPixmap(const QPixmap &source)
{

    // do nothing for invalid pixmaps
    if (source.isNull()) return 0;

    /*
    in some cases, pixmap handle is invalid. This is the case notably
    when Qt uses to RasterEngine. In this case, we create an X11 Pixmap
    explicitly and draw the source pixmap on it.
    */

#if HAVE_X11

    if (!m_isX11) {
        return 0;
    }

    // check connection
    if (!_connection) _connection = QX11Info::connection();

    const int width(source.width());
    const int height(source.height());

    // create X11 pixmap
    Pixmap pixmap = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), width, height, 32);

    // check gc
    if (!_gc) {
        _gc = xcb_generate_id(_connection);
        xcb_create_gc(_connection, _gc, pixmap, 0, 0x0);
    }

//         // create explicitly shared QPixmap from it
//         QPixmap dest( QPixmap::fromX11Pixmap( pixmap, QPixmap::ExplicitlyShared ) );
//
//         // create surface for pixmap
//         {
//             QPainter painter( &dest );
//             painter.setCompositionMode( QPainter::CompositionMode_Source );
//             painter.drawPixmap( 0, 0, source );
//         }
//
//
//         return pixmap;
    QImage image(source.toImage());
    xcb_put_image(
        _connection, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, _gc,
        image.width(), image.height(), 0, 0,
        0, 32,
        image.byteCount(), image.constBits());

    return (Qt::HANDLE)pixmap;

#else
    return 0;
#endif

}

void PanelShadows::Private::initPixmap(const QString &element)
{
    m_shadowPixmaps << q->pixmap(element);
}

QPixmap PanelShadows::Private::initEmptyPixmap(const QSize &size)
{
#if HAVE_X11

    if (!m_isX11) {
        return QPixmap();
    }

    QPixmap tempEmptyPix(size);

    if (!size.isEmpty()) {
        tempEmptyPix.fill(Qt::transparent);
    }

    return tempEmptyPix;
#else
    Q_UNUSED(size)
    return QPixmap();
#endif
}

void PanelShadows::Private::setupPixmaps()
{
    clearPixmaps();
    initPixmap(QStringLiteral("shadow-top"));
    initPixmap(QStringLiteral("shadow-topright"));
    initPixmap(QStringLiteral("shadow-right"));
    initPixmap(QStringLiteral("shadow-bottomright"));
    initPixmap(QStringLiteral("shadow-bottom"));
    initPixmap(QStringLiteral("shadow-bottomleft"));
    initPixmap(QStringLiteral("shadow-left"));
    initPixmap(QStringLiteral("shadow-topleft"));

    m_emptyCornerPix = initEmptyPixmap(QSize(1, 1));
    m_emptyCornerLeftPix = initEmptyPixmap(QSize(q->elementSize(QStringLiteral("shadow-topleft")).width(), 1));
    m_emptyCornerTopPix = initEmptyPixmap(QSize(1, q->elementSize(QStringLiteral("shadow-topleft")).height()));
    m_emptyCornerRightPix = initEmptyPixmap(QSize(q->elementSize(QStringLiteral("shadow-bottomright")).width(), 1));
    m_emptyCornerBottomPix = initEmptyPixmap(QSize(1, q->elementSize(QStringLiteral("shadow-bottomright")).height()));
    m_emptyVerticalPix = initEmptyPixmap(QSize(1, q->elementSize(QStringLiteral("shadow-left")).height()));
    m_emptyHorizontalPix = initEmptyPixmap(QSize(q->elementSize(QStringLiteral("shadow-top")).width(), 1));

    if (m_wayland.shmPool) {
        for (auto it = m_shadowPixmaps.constBegin(); it != m_shadowPixmaps.constEnd(); ++it) {
            m_wayland.shadowBuffers << m_wayland.shmPool->createBuffer(it->toImage());
        }
    }
}


void PanelShadows::Private::setupData(Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#if HAVE_X11

    if (!m_isX11) {
        return;
    }

    //shadow-top
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[0]));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyHorizontalPix));
    }

    //shadow-topright
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[1]));
    } else if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerTopPix));
    } else if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerRightPix));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerPix));
    }

    //shadow-right
    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[2]));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyVerticalPix));
    }

    //shadow-bottomright
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[3]));
    } else if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerBottomPix));
    } else if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerRightPix));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerPix));
    }

    //shadow-bottom
    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[4]));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyHorizontalPix));
    }

    //shadow-bottomleft
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[5]));
    } else if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerBottomPix));
    } else if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerLeftPix));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerPix));
    }

    //shadow-left
    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[6]));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyVerticalPix));
    }

    //shadow-topleft
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_shadowPixmaps[7]));
    } else if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerTopPix));
    } else if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerLeftPix));
    } else {
        data[enabledBorders] << reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerPix));
    }

#endif

    int left, top, right, bottom = 0;

    QSize marginHint;

    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-top-margin"));

        if (marginHint.isValid()) {
            top = marginHint.height();
        } else {
            top = m_shadowPixmaps[0].height(); // top
        }
    } else {
        top = 1;
    }

    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-right-margin"));

        if (marginHint.isValid()) {
            right = marginHint.width();
        } else {
            right = m_shadowPixmaps[2].width(); // right
        }
    } else {
        right = 1;
    }

    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-bottom-margin"));

        if (marginHint.isValid()) {
            bottom = marginHint.height();
        } else {
            bottom = m_shadowPixmaps[4].height(); // bottom
        }
    } else {
        bottom = 1;
    }

    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-left-margin"));

        if (marginHint.isValid()) {
            left = marginHint.width();
        } else {
            left = m_shadowPixmaps[6].width(); // left
        }
    } else {
        left = 1;
    }

    data[enabledBorders] << top << right << bottom << left;
}

void PanelShadows::Private::freeX11Pixmaps()
{
#if HAVE_X11

    if (!m_isX11) {
        return;
    }

    auto *display = QX11Info::display();

    if (!display) {
        return;
    }

    foreach (const QPixmap &pixmap, m_shadowPixmaps) {
        if (!pixmap.isNull()) {
            XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(pixmap)));
        }
    }

    if (!m_emptyCornerPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerPix)));
    }

    if (!m_emptyCornerBottomPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerBottomPix)));
    }

    if (!m_emptyCornerLeftPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerLeftPix)));
    }

    if (!m_emptyCornerRightPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerRightPix)));
    }

    if (!m_emptyCornerTopPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyCornerTopPix)));
    }

    if (!m_emptyVerticalPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyVerticalPix)));
    }

    if (!m_emptyHorizontalPix.isNull()) {
        XFreePixmap(display, reinterpret_cast<unsigned long>(createPixmap(m_emptyHorizontalPix)));
    }

#endif
}

void PanelShadows::Private::clearPixmaps()
{
#if HAVE_X11
    freeX11Pixmaps();

    m_emptyCornerPix = QPixmap();
    m_emptyCornerBottomPix = QPixmap();
    m_emptyCornerLeftPix = QPixmap();
    m_emptyCornerRightPix = QPixmap();
    m_emptyCornerTopPix = QPixmap();
    m_emptyVerticalPix = QPixmap();
    m_emptyHorizontalPix = QPixmap();
#endif
    freeWaylandBuffers();
    m_shadowPixmaps.clear();
    data.clear();
}

void PanelShadows::Private::freeWaylandBuffers()
{
    m_wayland.shadowBuffers.clear();
}

void PanelShadows::Private::updateShadow(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#if HAVE_X11

    if (m_isX11) {
        updateShadowX11(window, enabledBorders);
    }

#endif

    if (m_wayland.manager) {
        updateShadowWayland(window, enabledBorders);
    }
}

void PanelShadows::Private::updateShadowX11(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#if HAVE_X11

    if (m_shadowPixmaps.isEmpty()) {
        setupPixmaps();
    }

    if (!data.contains(enabledBorders)) {
        setupData(enabledBorders);
    }

    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);

//     qDebug() << "going to set the shadow of" << window->winId() << "to" << data;
    XChangeProperty(dpy, window->winId(), atom, XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<const unsigned char *>(data[enabledBorders].constData()), data[enabledBorders].size());
#endif
}

void PanelShadows::Private::updateShadowWayland(const QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
    if (!m_wayland.shmPool) {
        return;
    }

    if (m_wayland.shadowBuffers.isEmpty()) {
        setupPixmaps();
    }

    // TODO: check whether the surface already has a shadow
    KWayland::Client::Surface *surface = KWayland::Client::Surface::fromWindow(const_cast<QWindow *>(window));

    if (!surface) {
        return;
    }

    auto shadow = m_wayland.manager->createShadow(surface, surface);

    //shadow-top
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        shadow->attachTop(m_wayland.shadowBuffers.at(0));
    }

    //shadow-topright
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        shadow->attachTopRight(m_wayland.shadowBuffers.at(1));
    }

    //shadow-right
    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        shadow->attachRight(m_wayland.shadowBuffers.at(2));
    }

    //shadow-bottomright
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        shadow->attachBottomRight(m_wayland.shadowBuffers.at(3));
    }

    //shadow-bottom
    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        shadow->attachBottom(m_wayland.shadowBuffers.at(4));
    }

    //shadow-bottomleft
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        shadow->attachBottomLeft(m_wayland.shadowBuffers.at(5));
    }

    //shadow-left
    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        shadow->attachLeft(m_wayland.shadowBuffers.at(6));
    }

    //shadow-topleft
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        shadow->attachTopLeft(m_wayland.shadowBuffers.at(7));
    }

    QSize marginHint;
    QMarginsF margins = QMarginsF(1, 1, 1, 1);

    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-top-margin"));

        if (marginHint.isValid()) {
            margins.setTop(marginHint.height());
        } else {
            margins.setTop(m_shadowPixmaps[0].height());
        }
    }

    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-right-margin"));

        if (marginHint.isValid()) {
            margins.setRight(marginHint.width());
        } else {
            margins.setRight(m_shadowPixmaps[2].width());
        }
    }

    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-bottom-margin"));

        if (marginHint.isValid()) {
            margins.setBottom(marginHint.height());
        } else {
            margins.setBottom(m_shadowPixmaps[4].height());
        }
    }

    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        marginHint = q->elementSize(QStringLiteral("shadow-hint-left-margin"));

        if (marginHint.isValid()) {
            margins.setLeft(marginHint.width());
        } else {
            margins.setLeft(m_shadowPixmaps[6].width());
        }
    }

    shadow->setOffsets(margins);
    shadow->commit();
    surface->commit(KWayland::Client::Surface::CommitFlag::None);
}

void PanelShadows::Private::clearShadow(const QWindow *window)
{
    if (!static_cast<const QSurface *>(window)->surfaceHandle()) {
        qWarning() << "Cannot clear shadow from window without native surface!";
        return;
    }

#if HAVE_X11

    if (m_isX11) {
        clearShadowX11(window);
    }

#endif

    if (m_wayland.manager) {
        clearShadowWayland(window);
    }
}

void PanelShadows::Private::clearShadowX11(const QWindow *window)
{
#if HAVE_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);
    XDeleteProperty(dpy, window->winId(), atom);
#endif
}

void PanelShadows::Private::clearShadowWayland(const QWindow *window)
{
    KWayland::Client::Surface *surface = KWayland::Client::Surface::fromWindow(const_cast<QWindow *>(window));

    if (!surface) {
        return;
    }

    m_wayland.manager->removeShadow(surface);
    surface->commit(KWayland::Client::Surface::CommitFlag::None);
}

bool PanelShadows::enabled() const
{
    return hasElement(QStringLiteral("shadow-left"));
}

void PanelShadows::Private::setupWaylandIntegration()
{
    if (!KWindowSystem::isPlatformWayland()) {
        return;
    }

    using namespace KWayland::Client;
    ConnectionThread *connection = ConnectionThread::fromApplication(q);

    if (!connection) {
        return;
    }

    Registry *registry = new Registry(q);
    registry->create(connection);
    connect(registry, &Registry::shadowAnnounced, q,
    [this, registry](quint32 name, quint32 version) {
        m_wayland.manager = registry->createShadowManager(name, version, q);
        updateShadows();
    }, Qt::QueuedConnection
           );
    connect(registry, &Registry::shmAnnounced, q,
    [this, registry](quint32 name, quint32 version) {
        m_wayland.shmPool = registry->createShmPool(name, version, q);
        updateShadows();
    }, Qt::QueuedConnection
           );
    registry->setup();
    connection->roundtrip();
}

#include "moc_panelshadows_p.cpp"
