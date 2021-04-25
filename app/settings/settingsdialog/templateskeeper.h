/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef SETTINGSPARTTEMPLATESKEEPER_H
#define SETTINGSPARTTEMPLATESKEEPER_H

// local
#include "../../data/viewdata.h"
#include "../../data/viewstable.h"

// Qt
#include <QObject>

namespace Latte {
class CentralLayout;
class Corona;
namespace Settings {
namespace Controller {
class Layouts;
}
}
}

namespace Latte {
namespace Settings {
namespace Part {

class TemplatesKeeper : public QObject
{
    Q_OBJECT

public:
    explicit TemplatesKeeper(Settings::Controller::Layouts *parent, Latte::Corona *corona);
    ~TemplatesKeeper();

    QString storedView(const QString &layoutCurrentId, const QString &viewId);


public slots:
    void clear();

private:
    QString viewKeeperId(const QString &layoutCurrentId, const QString &viewId);

private:
    Latte::Data::ViewsTable m_storedViews;

    Latte::Corona *m_corona{nullptr};
    Settings::Controller::Layouts *m_layoutsController{nullptr};

    QList<CentralLayout *> m_garbageLayouts;
};

}
}
}

#endif
