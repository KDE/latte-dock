/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMASCREENGEOMETRIES_H
#define PLASMASCREENGEOMETRIES_H

// local
#include <coretypes.h>

// Qt
#include <QDBusServiceWatcher>
#include <QHash>
#include <QObject>
#include <QTimer>


namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace PlasmaExtended {

class ScreenGeometries: public QObject
{
    Q_OBJECT

public:
    ScreenGeometries(Latte::Corona *parent);
    ~ScreenGeometries() override;

private slots:
    void availableScreenGeometryChangedFrom(Latte::View *origin);

    void init();
    void updateGeometries();
    void clearGeometries();

    void onBroadcastToPlasmaChanged();

private slots:
    bool screenIsActive(const QString &screenName) const;
    void setPlasmaAvailableScreenRect(const QString &screenName, const QRect &rect);
    void setPlasmaAvailableScreenRegion(const QString &screenName, const QRegion &region);

private:
    bool m_plasmaInterfaceAvailable{false};

    //! this is needed in order to avoid too many costly calculations for available screen geometries
    QTimer m_publishTimer;

    //! this is needed in order to check if Plasma>=5.18 is running
    QTimer m_startupInitTimer;

    Latte::Corona *m_corona{nullptr};

    QList<Latte::Types::Visibility> m_ignoreModes{
        Latte::Types::AutoHide,
        Latte::Types::SidebarOnDemand,
        Latte::Types::SidebarAutoHide
    };

    QStringList m_lastScreenNames;

    QHash<QString, QRect> m_lastAvailableRect;
    QHash<QString, QRegion> m_lastAvailableRegion;

    QDBusServiceWatcher *m_plasmaServiceWatcher{nullptr};
};

}
}

#endif
