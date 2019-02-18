/*
 * Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backgroundtracker.h"

#include "plasma/extended/backgroundcache.h"

namespace Latte {

BackgroundTracker::BackgroundTracker(QObject *parent)
    : QObject(parent)
{
    m_cache = PlasmaExtended::BackgroundCache::self();

    connect(this, &BackgroundTracker::activityChanged, this, &BackgroundTracker::update);
    connect(this, &BackgroundTracker::locationChanged, this, &BackgroundTracker::update);
    connect(this, &BackgroundTracker::screenNameChanged, this, &BackgroundTracker::update);

    connect(m_cache, &PlasmaExtended::BackgroundCache::backgroundChanged, this, &BackgroundTracker::backgroundChanged);
}

BackgroundTracker::~BackgroundTracker()
{
}

bool BackgroundTracker::isBusy() const
{
    return m_busy;
}

int BackgroundTracker::location() const
{
    return m_location;
}

void BackgroundTracker::setLocation(int location)
{
    Plasma::Types::Location pLocation = static_cast<Plasma::Types::Location>(location);

    if (m_location == pLocation) {
        return;
    }

    m_location = pLocation;

    emit locationChanged();
}

float BackgroundTracker::currentBrightness() const
{
    return m_brightness;
}

QString BackgroundTracker::activity() const
{
    return m_activity;
}

void BackgroundTracker::setActivity(QString id)
{
    if (m_activity == id) {
        return;
    }

    m_activity = id;

    emit activityChanged();
}

QString BackgroundTracker::screenName() const
{
    return m_screenName;
}

void BackgroundTracker::setScreenName(QString name)
{
    if (m_screenName == name) {
        return;
    }

    m_screenName = name;

    emit screenNameChanged();
}

void BackgroundTracker::backgroundChanged(const QString &activity, const QString &screenName)
{
    if (m_activity==activity && m_screenName==screenName) {
        update();
    }
}

void BackgroundTracker::update()
{
    if (m_activity.isEmpty() || m_screenName.isEmpty()) {
        return;
    }

    m_brightness = m_cache->brightnessFor(m_activity, m_screenName, m_location);
    m_busy = m_cache->busyFor(m_activity, m_screenName, m_location);

    emit currentBrightnessChanged();
    emit isBusyChanged();
}

void BackgroundTracker::setBackgroundFromBroadcast(QString activity, QString screen, QString filename)
{
    m_cache->setBackgroundFromBroadcast(activity, screen, filename);
}

void BackgroundTracker::setBroadcastedBackgroundsEnabled(QString activity, QString screen, bool enabled)
{
    m_cache->setBroadcastedBackgroundsEnabled(activity, screen, enabled);
}

}
