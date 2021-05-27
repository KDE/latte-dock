/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VIEWINDICATORRESOURCES_H
#define VIEWINDICATORRESOURCES_H

// Qt
#include <QObject>

namespace Latte {
namespace ViewPart {
class Indicator;
}
}

namespace Latte {
namespace ViewPart {
namespace IndicatorPart {

/**
 * Resources requested from indicator in order to reduce consumption
 **/

class Resources: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> svgs READ svgs NOTIFY svgsChanged)

public:
    Resources(Indicator *parent);
    virtual ~Resources();

    QList<QObject *> svgs() const;

public slots:
    Q_INVOKABLE void setSvgImagePaths(QStringList paths);

signals:
    void svgsChanged();

private:
    QStringList m_svgImagePaths;

    Indicator *m_indicator{nullptr};

    QList<QObject *> m_svgs;
};

}
}
}

#endif
