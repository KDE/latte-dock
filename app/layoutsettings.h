/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
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

#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H

#include <QObject>

#include <KConfigGroup>
#include <KSharedConfig>

#include "dockcorona.h"

class DockCorona;

namespace Latte {

//! This class is responsible to hold the settings for a specific layout.
//! It also updates always the relevant layout configuration concerning
//! its general settings (no the containments)
class LayoutSettings : public QObject {
    Q_OBJECT

public:
    LayoutSettings(QObject *parent, QString layoutFile);
    LayoutSettings(QObject *parent, KSharedConfig::Ptr config);
    ~LayoutSettings() override;

private:
    QString m_layoutFile;

    DockCorona *m_corona{nullptr};
    KConfigGroup m_layoutGroup;
};

}

#endif // LAYOUTSETTINGS_H
