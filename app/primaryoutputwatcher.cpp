/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "primaryoutputwatcher.h"

#include <KWindowSystem>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include "qwayland-kde-primary-output-v1.h"
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>

#include <config-latte.h>
#if HAVE_X11
#include <QTimer> //Used only in x11 case
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#endif

class WaylandPrimaryOutput : public QObject, public QtWayland::kde_primary_output_v1
{
    Q_OBJECT
public:
    WaylandPrimaryOutput(struct ::wl_registry *registry, int id, int version, QObject *parent)
        : QObject(parent)
        , QtWayland::kde_primary_output_v1(registry, id, version)
    {
    }

    void kde_primary_output_v1_primary_output(const QString &outputName) override
    {
        Q_EMIT primaryOutputChanged(outputName);
    }

Q_SIGNALS:
    void primaryOutputChanged(const QString &outputName);
};

PrimaryOutputWatcher::PrimaryOutputWatcher(QObject *parent)
    : QObject(parent)
{
#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        m_primaryOutputName = qGuiApp->primaryScreen()->name();
        qGuiApp->installNativeEventFilter(this);
        const xcb_query_extension_reply_t *reply = xcb_get_extension_data(QX11Info::connection(), &xcb_randr_id);
        m_xrandrExtensionOffset = reply->first_event;
        setPrimaryOutputName(qGuiApp->primaryScreen()->name());
        connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen *newPrimary) {
            setPrimaryOutputName(newPrimary->name());
        });
    }
#endif
    if (KWindowSystem::isPlatformWayland()) {
        setupRegistry();
    }
}

void PrimaryOutputWatcher::setPrimaryOutputName(const QString &newOutputName)
{
    if (newOutputName != m_primaryOutputName) {
        const QString oldOutputName = m_primaryOutputName;
        m_primaryOutputName = newOutputName;
        Q_EMIT primaryOutputNameChanged(oldOutputName, newOutputName);
    }
}

void PrimaryOutputWatcher::setupRegistry()
{
    auto m_connection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!m_connection) {
        return;
    }

    // Asking for primaryOutputName() before this happened, will return qGuiApp->primaryScreen()->name() anyways, so set it so the primaryOutputNameChange will
    // have parameters that are coherent
    m_primaryOutputName = qGuiApp->primaryScreen()->name();
    m_registry = new KWayland::Client::Registry(this);
    connect(m_registry, &KWayland::Client::Registry::interfaceAnnounced, this, [this](const QByteArray &interface, quint32 name, quint32 version) {
        if (interface == WaylandPrimaryOutput::interface()->name) {
            auto m_outputManagement = new WaylandPrimaryOutput(m_registry->registry(), name, version, this);
            connect(m_outputManagement, &WaylandPrimaryOutput::primaryOutputChanged, this, [this](const QString &outputName) {
                m_primaryOutputWayland = outputName;
                // Only set the outputName when there's a QScreen attached to it
                if (screenForName(outputName)) {
                    setPrimaryOutputName(outputName);
                }
            });
        }
    });

    // In case the outputName was received before Qt reported the screen
    connect(qGuiApp, &QGuiApplication::screenAdded, this, [this](QScreen *screen) {
        if (screen->name() == m_primaryOutputWayland) {
            setPrimaryOutputName(m_primaryOutputWayland);
        }
    });

    m_registry->create(m_connection);
    m_registry->setup();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool PrimaryOutputWatcher::nativeEventFilter(const QByteArray &eventType, void *message, long int *result)
#else
bool PrimaryOutputWatcher::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#endif
{
    Q_UNUSED(result);
#if HAVE_X11
    // a particular edge case: when we switch the only enabled screen
    // we don't have any signal about it, the primary screen changes but we have the same old QScreen* getting recycled
    // see https://bugs.kde.org/show_bug.cgi?id=373880
    // if this slot will be invoked many times, their//second time on will do nothing as name and primaryOutputName will be the same by then
    if (eventType[0] != 'x') {
        return false;
    }

    xcb_generic_event_t *ev = static_cast<xcb_generic_event_t *>(message);

    const auto responseType = XCB_EVENT_RESPONSE_TYPE(ev);

    if (responseType == m_xrandrExtensionOffset + XCB_RANDR_SCREEN_CHANGE_NOTIFY) {
        QTimer::singleShot(0, this, [this]() {
            setPrimaryOutputName(qGuiApp->primaryScreen()->name());
        });
    }
#endif
    return false;
}

QScreen *PrimaryOutputWatcher::screenForName(const QString &outputName) const
{
    const auto screens = qGuiApp->screens();
    for (auto screen : screens) {
        if (screen->name() == outputName) {
            return screen;
        }
    }
    return nullptr;
}

QScreen *PrimaryOutputWatcher::primaryScreen() const
{
    auto screen = screenForName(m_primaryOutputName);
    if (!screen) {
        qDebug() << "PrimaryOutputWatcher: Could not find primary screen:" << m_primaryOutputName;
        return qGuiApp->primaryScreen();
    }
    return screen;
}

#include "primaryoutputwatcher.moc"

