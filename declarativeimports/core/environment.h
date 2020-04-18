/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef LATTEENVIRONMENT_H
#define LATTEENVIRONMENT_H

// Qt
#include <QObject>
#include <QQmlEngine>
#include <QJSEngine>


namespace Latte{

class Environment final: public QObject
{
    Q_OBJECT

    Q_PROPERTY(int separatorLength READ separatorLength CONSTANT)

    Q_PROPERTY(uint shortDuration READ shortDuration NOTIFY shortDurationChanged)
    Q_PROPERTY(uint longDuration READ longDuration NOTIFY longDurationChanged)

    Q_PROPERTY(uint frameworksVersion READ frameworksVersion NOTIFY frameworksVersionChanged)
    Q_PROPERTY(uint plasmaDesktopVersion READ plasmaDesktopVersion NOTIFY plasmaDesktopVersionChanged)

public:
    static const int SeparatorLength = 6;

    explicit Environment(QObject *parent = nullptr);

    int separatorLength() const;

    uint shortDuration() const;
    uint longDuration() const;

    uint frameworksVersion() const;
    uint plasmaDesktopVersion();

public slots:
    Q_INVOKABLE uint makeVersion(uint major, uint minor, uint release) const;

signals:
    void frameworksVersionChanged();
    void longDurationChanged();
    void plasmaDesktopVersionChanged();
    void shortDurationChanged();

private:
    void loadPlasmaDesktopVersion();

    uint identifyPlasmaDesktopVersion();

private:
    int m_plasmaDesktopVersion{ -1};

};

static QObject *environment_qobject_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

// NOTE: QML engine is the owner of this resource
    return new Environment;
}

}

#endif
