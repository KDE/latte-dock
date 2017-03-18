/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include "dockcorona.h"
#include "../liblattedock/dock.h"

#include <KConfigGroup>
#include <KSharedConfig>

class DockCorona;

namespace Latte {

class GlobalSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool exposeAltSession READ exposeAltSession WRITE setExposeAltSession NOTIFY exposeAltSessionChanged)

    Q_PROPERTY(QAction *altSessionAction READ altSessionAction NOTIFY altSessionActionChanged)

public:
    GlobalSettings(QObject *parent = nullptr);
    ~GlobalSettings() override;

    void load();

    bool exposeAltSession() const;
    void setExposeAltSession(bool state);

    QAction *altSessionAction() const;

signals:
    void altSessionActionChanged();
    void exposeAltSessionChanged();

private slots:
    void currentSessionChanged(Dock::SessionType type);
    void enableAltSession(bool enabled);

private:
    void save();

    bool m_exposeAltSession{false};
    QAction *m_altSessionAction{nullptr};
    DockCorona *m_corona{nullptr};

    KConfigGroup m_configGroup;
};

}

#endif // GLOBALSETTINGS_H
