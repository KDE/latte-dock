/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SCHEMESTRACKER_H
#define SCHEMESTRACKER_H

// local
#include "../windowinfowrap.h"

// Qt
#include <QObject>


namespace Latte {
namespace WindowSystem {
class AbstractWindowInterface;
class SchemeColors;
}
}

namespace Latte {
namespace WindowSystem {
namespace Tracker {

class Schemes : public QObject {
    Q_OBJECT

public:
    Schemes(AbstractWindowInterface *parent);
    ~Schemes() override;

    SchemeColors *schemeForWindow(WindowId wId);
    void setColorSchemeForWindow(WindowId wId, QString scheme);

    SchemeColors *schemeForFile(const QString &scheme);

signals:
    void colorSchemeChanged(const WindowId &wid);
    void defaultSchemeChanged();

private slots:
    void updateDefaultScheme();

private:
    void init();

private:
     AbstractWindowInterface *m_wm;

     //! scheme file and its loaded colors
     QMap<QString, Latte::WindowSystem::SchemeColors *> m_schemes;

     //! window id and its corresponding scheme file
     QMap<WindowId, QString> m_windowScheme;
};

}
}
}

#endif
