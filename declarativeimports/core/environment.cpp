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

uint Environment::makeVersion(uint major, uint minor, uint release) const
{
    return (((major) << 16) | ((minor) << 8) | (release));
}

}
