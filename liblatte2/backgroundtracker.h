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

#ifndef BACKGROUNDTRACKER_H
#define BACKGROUNDTRACKER_H

// local
#include "plasma/extended/backgroundcache.h"

// Qt
#include <QObject>

namespace Latte{

class BackgroundTracker: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString activity READ activity WRITE setActivity NOTIFY activityChanged)
    Q_PROPERTY(QString screenName READ screenName WRITE setScreenName NOTIFY screenNameChanged)

public:
    BackgroundTracker(QObject *parent = nullptr);
    virtual ~BackgroundTracker();

    QString activity() const;
    void setActivity(QString id);

    QString screenName() const;
    void setScreenName(QString name);

signals:
    void activityChanged();
    void screenNameChanged();

private:
    QString m_activity;
    QString m_screenName;

    PlasmaExtended::BackgroundCache *m_cache{nullptr};
};

}

#endif
