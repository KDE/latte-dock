/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    static const int SeparatorLength = 5;

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
