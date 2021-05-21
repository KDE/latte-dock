/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LATTEPACKAGE_H
#define LATTEPACKAGE_H

// Qt
#include <QObject>

// KDE
#include <KPackage/PackageStructure>

namespace Latte {
class Package : public KPackage::PackageStructure
{
    Q_OBJECT

public:
    explicit Package(QObject *parent = 0, const QVariantList &args = QVariantList());

    ~Package() override;
    void initPackage(KPackage::Package *package) override;
    void pathChanged(KPackage::Package *package) override;
};

}
#endif // LATTEPACKAGE_H
