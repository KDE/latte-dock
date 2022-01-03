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
    qDebug() << " PLASMA STRUTS MANAGER :: checking availability....";
    bool serviceavailable{false};

    if (QDBusConnection::sessionBus().interface()) {
        serviceavailable = QDBusConnection::sessionBus().interface()->isServiceRegistered(PLASMASERVICE).value();
        qDebug() << "PLASMA STRUTS MANAGER :: interface availability :: " << QDBusConnection::sessionBus().interface()->isServiceRegistered(PLASMASERVICE).value();
    }

    connect(m_corona->universalSettings(), &Latte::UniversalSettings::isAvailableGeometryBroadcastedToPlasmaChanged, this, &ScreenGeometries::onBroadcastToPlasmaChanged);

    if (serviceavailable) {
        m_plasmaInterfaceAvailable = true;

        qDebug() << " PLASMA STRUTS MANAGER :: is available...";

        connect(m_corona, &Latte::Corona::availableScreenRectChangedFrom, this, &ScreenGeometries::availableScreenGeometryChangedFrom);
        connect(m_corona, &Latte::Corona::availableScreenRegionChangedFrom, this, &ScreenGeometries::availableScreenGeometryChangedFrom);

        connect(m_corona->layoutsManager()->synchronizer(), &Latte::Layouts::Synchronizer::centralLayoutsChanged, this, [&]() {
            m_publishTimer.start();
        });

        connect(m_corona->activitiesConsumer(), &KActivities::Consumer::currentActivityChanged, this, [&]() {
            if (m_corona->universalSettings()->isAvailableGeometryBroadcastedToPlasma()) {
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

void ScreenGeometries::setPlasmaAvailableScreenRect(const QString &screenName, const QRect &rect)
{
    QDBusMessage message = QDBusMessage::createMethodCall(PLASMASERVICE,
                                                          "/StrutManager",
                                                          PLASMASTRUTNAMESPACE,
                                                          "setAvailableScreenRect");
    QVariantList args;

    args << LATTESERVICE
         << screenName
         << rect;

    message.setArguments(args);
    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

void ScreenGeometries::setPlasmaAvailableScreenRegion(const QString &screenName, const QRegion &region)
{
    QDBusMessage message = QDBusMessage::createMethodCall(PLASMASERVICE,
                                                          "/StrutManager",
                                                          PLASMASTRUTNAMESPACE,
                                                          "setAvailableScreenRegion");

    QVariant regionvariant;

    QList<QRect> rects;
    if (!region.isNull()) {
        //! transorm QRegion to QList<QRect> in order to be sent through dbus
        foreach (const QRect &rect, region) {
            rects << rect;
        }
    } else {
        rects << QRect();
    }
    regionvariant = QVariant::fromValue(rects);

    QVariantList args;

    args << LATTESERVICE
         << screenName
         << regionvariant;

    message.setArguments(args);
    QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
}

void ScreenGeometries::clearGeometries()
{
    if (!m_plasmaInterfaceAvailable) {
        return;
    }

    for (QScreen *screen : qGuiApp->screens()) {
        QString scrName = screen->name();
        int scrId = m_corona->screenPool()->id(screen->name());

        if (m_corona->screenPool()->hasScreenId(scrId)) {
            setPlasmaAvailableScreenRect(scrName, QRect());
            setPlasmaAvailableScreenRegion(scrName, QRegion());
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

    QStringList availableScreenNames;

    qDebug() << " PLASMA SCREEN GEOMETRIES, LAST AVAILABLE SCREEN RECTS :: " << m_lastAvailableRect;

    QStringList clearedScreenNames;

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

            bool clearedScreen = (availableRect == screen->geometry());

            if (!clearedScreen) {
                //! Disable checks because of the workaround concerning plasma desktop behavior
                if (!m_lastAvailableRect.contains(scrName) || m_lastAvailableRect[scrName] != availableRect) {
                    m_lastAvailableRect[scrName] = availableRect;
                    setPlasmaAvailableScreenRect(scrName, availableRect);
                    qDebug() << " PLASMA SCREEN GEOMETRIES, AVAILABLE RECT :: " << screen->name() << " : " << availableRect;
                }

                if (!m_lastAvailableRegion.contains(scrName) || m_lastAvailableRegion[scrName] != availableRegion) {
                    m_lastAvailableRegion[scrName] = availableRegion;
                    setPlasmaAvailableScreenRegion(scrName, availableRegion);
                    qDebug() << " PLASMA SCREEN GEOMETRIES, AVAILABLE REGION :: " << screen->name() << " : " << availableRegion;
                }
            } else {
                clearedScreenNames << scrName;
            }
        }

        availableScreenNames << scrName;
    }

    //! check for inactive screens that were published previously
    for (QString &lastScrName : m_lastScreenNames) {
        bool scractive = screenIsActive(lastScrName);

        if (!scractive || clearedScreenNames.contains(lastScrName)) {
            //! screen became inactive and its geometries could be unpublished
            setPlasmaAvailableScreenRect(lastScrName, QRect());
            setPlasmaAvailableScreenRegion(lastScrName, QRegion());

            m_lastAvailableRect.remove(lastScrName);
            m_lastAvailableRegion.remove(lastScrName);
        }

        if (!scractive) {
            qDebug() << " PLASMA SCREEN GEOMETRIES, INACTIVE SCREEN :: " << lastScrName;
        } else if (clearedScreenNames.contains(lastScrName)) {
            qDebug() << " PLASMA SCREEN GEOMETRIES, CLEARED SCREEN :: " << lastScrName;
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
