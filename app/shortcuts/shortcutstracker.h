/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef SHORTCUTSTRACKER_H
#define SHORTCUTSTRACKER_H

// Qt
#include <QObject>

// KDE
#include <KSharedConfig>

namespace Latte {
namespace ShortcutsPart {

class ShortcutsTracker: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool basedOnPositionEnabled READ basedOnPositionEnabled NOTIFY badgesForActivateChanged)
    Q_PROPERTY(QStringList badgesForActivate READ badgesForActivate NOTIFY badgesForActivateChanged)

public:
    ShortcutsTracker(QObject *parent);
    ~ShortcutsTracker() override;

    void clearAllAppletShortcuts();

    bool basedOnPositionEnabled() const;

    QStringList badgesForActivate() const;

    QList<uint> appletsWithPlasmaShortcuts();

public slots:
    Q_INVOKABLE QString appletShortcutBadge(int appletId);

signals:
    void badgesForActivateChanged();

private slots:
    void shortcutsFileChanged(const QString &file);

private:
    void initGlobalShortcutsWatcher();
    //! access user set global shortcuts for activate entries
    void parseGlobalShortcuts();

    QString shortcutToBadge(QStringList shortcutRecords);

private:
    bool m_basedOnPositionEnabled{false};

    QStringList m_badgesForActivate;

    //! shortcuts assigned to applets through plasma infrastructure
    //! <applet id, shortcut>
    QHash<uint, QString> m_appletShortcuts;

    KSharedConfig::Ptr m_shortcutsConfigPtr;
};

}
}

#endif
