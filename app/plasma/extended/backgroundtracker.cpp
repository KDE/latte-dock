/*
    SPDX-FileCopyrightText: 2018 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backgroundtracker.h"

// local
#include "plasma/extended/backgroundcache.h"

namespace Latte {

BackgroundTracker::BackgroundTracker(QObject *parent)
    : QObject(parent)
{
    connect(this, &BackgroundTracker::activityChanged, this, &BackgroundTracker::update);
    connect(this, &BackgroundTracker::locationChanged, this, &BackgroundTracker::update);
    connect(this, &BackgroundTracker::screenNameChanged, this, &BackgroundTracker::update);

    connect(PlasmaExtended::BackgroundCache::self(), &PlasmaExtended::BackgroundCache::backgroundChanged, this, &BackgroundTracker::backgroundChanged);
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

    m_brightness = PlasmaExtended::BackgroundCache::self()->brightnessFor(m_activity, m_screenName, m_location);
    m_busy = PlasmaExtended::BackgroundCache::self()->busyFor(m_activity, m_screenName, m_location);

    emit currentBrightnessChanged();
    emit isBusyChanged();
}

}
