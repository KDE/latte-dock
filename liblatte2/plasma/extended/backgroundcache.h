/*
*  Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef PLASMABACKGROUNDCACHE_H
#define PLASMABACKGROUNDCACHE_H

// local
#include "screenpool.h"

// Qt
#include <QHash>
#include <QObject>

// Plasma
#include <Plasma>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

struct imageHints {
    bool busy{false};
    float brightness{-1000};
};

typedef QHash<Plasma::Types::Location, imageHints> EdgesHash;

namespace Latte {
namespace PlasmaExtended {

class BackgroundCache: public QObject
{
    Q_OBJECT

public:
    static BackgroundCache *self();
    ~BackgroundCache() override;

    bool busyFor(QString activity, QString screen, Plasma::Types::Location location);
    float brightnessFor(QString activity, QString screen, Plasma::Types::Location location);

    QString background(QString activity, QString screen);

    void setBackgroundFromBroadcast(QString activity, QString screen, QString filename);
    void setBroadcastedBackgroundsEnabled(QString activity, QString screen, bool enabled);

signals:
    void backgroundChanged(const QString &activity, const QString &screenName);

private slots:
    void reload();
    void settingsFileChanged(const QString &file);

private:
    BackgroundCache(QObject *parent = nullptr);

    bool backgroundIsBroadcasted(QString activity, QString screenName);
    bool pluginExistsFor(QString activity, QString screenName);
    bool areaIsBusy(float bright1, float bright2);
    bool busyForFile(QString imageFile, Plasma::Types::Location location);
    bool isDesktopContainment(const KConfigGroup &containment) const;

    float brightnessForFile(QString imageFile, Plasma::Types::Location location);
    float brightnessFromArea(QImage &image, int firstRow, int firstColumn, int endRow, int endColumn);
    QString backgroundFromConfig(const KConfigGroup &config, QString wallpaperPlugin) const;

    void cleanupHashes();
    void updateImageCalculations(QString imageFile, Plasma::Types::Location location);

private:
    bool m_initialized{false};

    QString m_defaultWallpaperPath;

    ScreenPool *m_pool{nullptr};

    //! screen aware backgrounds: activity id, screen name, backgroundfile
    QHash<QString, QHash<QString, QString>> m_backgrounds;

    //! plugin names tracked: activity id, screen name, pluginName
    QHash<QString, QHash<QString, QString>> m_plugins;

    //! backgrounds that are broadcasted (not found through plasma files):
    //! and have higher priority: activity id, screen names
    QHash<QString, QList<QString>> m_broadcasted;

    //! image file and brightness per edge
    QHash<QString, EdgesHash> m_hintsCache;

    KSharedConfig::Ptr m_plasmaConfig;
};

}
}

#endif
