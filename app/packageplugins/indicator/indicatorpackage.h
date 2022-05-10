/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INDICATORPACKAGE_H
#define INDICATORPACKAGE_H

// Qt
#include <QObject>

// KDE
#include <KPackage/PackageStructure>

namespace Latte {
class IndicatorPackage : public KPackage::PackageStructure
{
    Q_OBJECT

public:
    explicit IndicatorPackage(QObject *parent, const QVariantList &args );

    void initPackage(KPackage::Package *package) override;
};

}
#endif // INDICATORPACKAGE_H
