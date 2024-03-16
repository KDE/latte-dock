/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "environment.h"

// Qt
#include <QDebug>
#include <QProcess>

// KF
#include <KCoreAddons>

// Plasma
#include <Plasma/plasma_version.h>

#define LONGDURATION 240
#define SHORTDURATION 40

namespace Latte{

const int Environment::SeparatorLength;

Environment::Environment(QObject *parent)
    : QObject(parent)
{
}

int Environment::separatorLength() const
{
    return SeparatorLength;
}

uint Environment::shortDuration() const
{
    return SHORTDURATION;
}

uint Environment::longDuration() const
{
    return LONGDURATION;
}

uint Environment::frameworksVersion() const
{
    return KCoreAddons::version();
}

uint Environment::plasmaDesktopVersion()
{
    if (m_plasmaDesktopVersion == -1) {
        m_plasmaDesktopVersion = identifyPlasmaDesktopVersion();
    }

    return m_plasmaDesktopVersion;
}

uint Environment::makeVersion(uint major, uint minor, uint release) const
{
    return (((major) << 16) | ((minor) << 8) | (release));
}

uint Environment::identifyPlasmaDesktopVersion()
{
    uint maj = PLASMA_VERSION_MAJOR;
    uint min = PLASMA_VERSION_MINOR;
    uint rel = PLASMA_VERSION_PATCH;

    if (maj > 0) {
        qDebug() << " /////////////////////////";
        uint desktopVersion = makeVersion(maj, min, rel);

        QString message("Plasma Desktop version:  " + QString::number(maj) + "."
                        + QString::number(min) + "." + QString::number(rel)
                        + " (" + QString::number(desktopVersion) + ")");
        qDebug() << message;
        qDebug() << " /////////////////////////";

        return desktopVersion;
    }

    qDebug() << " /////////////////////////";

    return 0;
}

}
