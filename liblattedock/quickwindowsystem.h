/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef QUICKWINDOWSYSTEM_H
#define QUICKWINDOWSYSTEM_H

#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>

namespace Latte {

/**
 * @brief The QuickWindowSystem class,
 * is a tiny class that provide basic information of WindowSystem
 */
class QuickWindowSystem final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool compositingActive READ compositingActive NOTIFY compositingChanged FINAL)
    Q_PROPERTY(bool isPlatformWayland READ isPlatformWayland NOTIFY isPlatformWaylandChanged FINAL)

    Q_PROPERTY(uint frameworksVersion READ frameworksVersion NOTIFY frameworksVersionChanged)
    Q_PROPERTY(uint plasmaDesktopVersion READ plasmaDesktopVersion NOTIFY plasmaDesktopVersionChanged)

public:
    explicit QuickWindowSystem(QObject *parent = nullptr);
    virtual ~QuickWindowSystem();

    bool compositingActive() const;
    bool isPlatformWayland() const;

    uint frameworksVersion() const;
    uint plasmaDesktopVersion();

public slots:
    Q_INVOKABLE uint makeVersion(uint major, uint minor, uint release) const;

signals:
    void compositingChanged();
    void frameworksVersionChanged();
    void isPlatformWaylandChanged();
    void plasmaDesktopVersionChanged();

private:
    void loadPlasmaDesktopVersion();

    uint identifyPlasmaDesktopVersion();

private:
    bool m_compositing{true};

    int m_plasmaDesktopVersion{ -1};
};

static QObject *windowsystem_qobject_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

// NOTE: QML engine is the owner of this resource
    return new QuickWindowSystem;
}

}

#endif // QUICKWINDOWSYSTEM_H
