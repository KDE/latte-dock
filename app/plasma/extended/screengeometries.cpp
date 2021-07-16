/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "screengeometries.h"

//!local
#include "../../lattecorona.h"
#include "../../screenpool.h"
#include "../../view/view.h"
#include "../../layout/genericlayout.h"
#include "../../layouts/manager.h"
#include "../../settings/universalsettings.h"

// Qt
#include <QDebug>
#include <QtDBus>


#define LATTESERVICE "org.kde.lattedock"
#define PLASMASERVICE "org.kde.plasmashell"
#define PLASMASTRUTNAMESPACE "org.kde.PlasmaShell.StrutManager"

#define PUBLISHINTERVAL 1000

namespace Latte {
namespace PlasmaExtended {

ScreenGeometries::ScreenGeometries(Latte::Corona *parent)
    : QObject(parent),
      m_corona(parent),
      m_plasmaServiceWatcher(new QDBusServiceWatcher(this))
{
    qDBusRegisterMetaType<QList<QRect>>();

    m_startupInitTimer.setInterval(2500);
    m_startupInitTimer.setSingleShot(true);
    connect(&m_startupInitTimer, &QTimer::timeout, this, &ScreenGeometries::init);

    m_publishTimer.setInterval(PUBLISHINTERVAL);
    m_publishTimer.setSingleShot(true);
    connect(&m_publishTimer, &QTimer::timeout, this, &ScreenGeometries::updateGeometries);

    m_startupInitTimer.start();

    m_plasmaServiceWatcher->setConnection(QDBusConnection::sessionBus());
    m_plasmaServiceWatcher->setWatchedServices(QStringList({PLASMASERVICE}));
    connect(m_plasmaServiceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [this](const QString & serviceName) {
        if (serviceName == PLASMASERVICE && !m_plasmaInterfaceAvailable) {
            init();
        }
    });
}

ScreenGeometries::~ScreenGeometries()
{
    qDebug() << "Plasma Extended Screen Geometries :: Deleted...";
}

void ScreenGeometries::init()
{
    QDBusInterface plasmaStrutsIface(PLASMASERVICE, "/StrutManager", PLASMASTRUTNAMESPACE, QDBusConnection::sessionBus());

    connect(m_corona->universalSettings(), &Latte::UniversalSettings::isAvailableGeometryBroadcastedToPlasmaChanged, this, &ScreenGeometries::onBroadcastToPlasmaChanged);

    if (plasmaStrutsIface.isValid()) {
        m_plasmaInterfaceAvailable = true;

        qDebug() << " PLASMA STRUTS MANAGER :: is available...";

        connect(m_corona, &Latte::Corona::availableScreenRectChangedFrom, this, &ScreenGeometries::availableScreenGeometryChangedFrom);
        connect(m_corona, &Latte::Corona::availableScreenRegionChangedFrom, this, &ScreenGeometries::availableScreenGeometryChangedFrom);

        connect(m_corona->layoutsManager()->synchronizer(), &Latte::Layouts::Synchronizer::centralLayoutsChanged, this, [&]() {
            m_publishTimer.start();
        });

        connect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
            if (m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma()) {
                m_forceGeometryBroadcast = true;
                m_publishTimer.start();
            }
        });

        if (m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma()) {
            m_publishTimer.start();
        }
    }
}

bool ScreenGeometries::screenIsActive(const QString &screenName) const
{
    for (QScreen *screen : qGuiApp->screens()) {
        if (screen->name() == screenName) {
            return true;
        }
    }

    return false;
}

void ScreenGeometries::onBroadcastToPlasmaChanged()
{
    if (m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma()) {
        m_publishTimer.start();
    } else {
        clearGeometries();
    }
}

void ScreenGeometries::clearGeometries()
{
    if (!m_plasmaInterfaceAvailable) {
        return;
    }

    QDBusInterface plasmaStrutsIface(PLASMASERVICE, "/StrutManager", PLASMASTRUTNAMESPACE, QDBusConnection::sessionBus());

    if (!plasmaStrutsIface.isValid()) {
        return;
    }

    for (QScreen *screen : qGuiApp->screens()) {
        QString scrName = screen->name();
        int scrId = m_corona->screenPool()->id(screen->name());

        if (m_corona->screenPool()->hasScreenId(scrId)) {
            plasmaStrutsIface.call("setAvailableScreenRect", LATTESERVICE, scrName, QRect());
            plasmaStrutsIface.call("setAvailableScreenRegion", LATTESERVICE, scrName, QVariant());
        }
    }

    m_lastAvailableRect.clear();
    m_lastAvailableRegion.clear();
}

void ScreenGeometries::updateGeometries()
{
    if (!m_plasmaInterfaceAvailable || !m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma()) {
        return;
    }

    QDBusInterface plasmaStrutsIface(PLASMASERVICE, "/StrutManager", PLASMASTRUTNAMESPACE, QDBusConnection::sessionBus());

    if (!plasmaStrutsIface.isValid()) {
        return;
    }

    QStringList availableScreenNames;

    qDebug() << " PLASMA SCREEN GEOMETRIES, LAST AVAILABLE SCREEN RECTS :: " << m_lastAvailableRect;

    //! check for available geometries changes
    for (QScreen *screen : qGuiApp->screens()) {
        QString scrName = screen->name();
        int scrId = m_corona->screenPool()->id(screen->name());

        qDebug() << " PLASMA SCREEN GEOMETRIES, SCREEN :: " << scrId << " - " << scrName;

        if (m_corona->screenPool()->hasScreenId(scrId)) {
            QRect availableRect = m_corona->availableScreenRectWithCriteria(scrId,
                                                                            QString(),
                                                                            m_ignoreModes,
                                                                            QList<Plasma::Types::Location>(),
                                                                            true,
                                                                            true);

            QRegion availableRegion = m_corona->availableScreenRegionWithCriteria(scrId,
                                                                                  QString(),
                                                                                  m_ignoreModes,
                                                                                  QList<Plasma::Types::Location>(),
                                                                                  true,
                                                                                  true);

            //! Workaround: Force update, to workaround Plasma not updating its layout at some cases
            //! Example: Canvas,Music activities use the same Layout. Unity activity
            //! is using a different layout. When the user from Unity is switching to
            //! Music and afterwards to Canvas the desktop elements are not positioned properly
            if (m_forceGeometryBroadcast) {
                plasmaStrutsIface.call("setAvailableScreenRect", LATTESERVICE, scrName, QRect());
            }

            //! Disable checks because of the workaround concerning plasma desktop behavior
            if (m_forceGeometryBroadcast || (!m_lastAvailableRect.contains(scrName) || m_lastAvailableRect[scrName] != availableRect)) {
                m_lastAvailableRect[scrName] = availableRect;
                plasmaStrutsIface.call("setAvailableScreenRect", LATTESERVICE, scrName, availableRect);
                qDebug() << " PLASMA SCREEN GEOMETRIES, AVAILABLE RECT :: " << screen->name() << " : " << availableRect;
            }

            if (m_forceGeometryBroadcast) {
                m_forceGeometryBroadcast = false;
            }

            if (!m_lastAvailableRegion.contains(scrName) || m_lastAvailableRegion[scrName] != availableRegion) {
                m_lastAvailableRegion[scrName] = availableRegion;

                //! transorm QRegion to QList<QRect> in order to be sent through dbus
                QList<QRect> rects;
                foreach (const QRect &rect, availableRegion) {
                    rects << rect;
                }

                plasmaStrutsIface.call("setAvailableScreenRegion", LATTESERVICE, scrName, QVariant::fromValue(rects));
                qDebug() << " PLASMA SCREEN GEOMETRIES, AVAILABLE REGION :: " << screen->name() << " : " << availableRegion;
            }
        }

        availableScreenNames << scrName;
    }

    //! check for inactive screens that were published previously
    for (QString &lastScrName : m_lastScreenNames) {
        if (!screenIsActive(lastScrName)) {
            //! screen became inactive and its geometries could be unpublished
            plasmaStrutsIface.call("setAvailableScreenRect", LATTESERVICE, lastScrName, QRect());
            plasmaStrutsIface.call("setAvailableScreenRegion", LATTESERVICE, lastScrName, QVariant::fromValue(QList<QRect>()));

            m_lastAvailableRect.remove(lastScrName);
            m_lastAvailableRegion.remove(lastScrName);
            qDebug() << " PLASMA SCREEN GEOMETRIES, INACTIVE SCREEN :: " << lastScrName;
        }
    }

    m_lastScreenNames = availableScreenNames;
}

void ScreenGeometries::availableScreenGeometryChangedFrom(Latte::View *origin)
{
    if (m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma() &&  origin && origin->layout() && origin->layout()->isCurrent()) {
        m_publishTimer.start();
    }
}

}
}
