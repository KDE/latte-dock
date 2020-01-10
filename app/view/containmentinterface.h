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

#ifndef VIEWCONTAINMENTINTERFACE_H
#define VIEWCONTAINMENTINTERFACE_H

// Qt
#include <QMetaMethod>
#include <QObject>
#include <QPointer>
#include <QQuickItem>

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace ViewPart {

class ContainmentInterface: public QObject
{
    Q_OBJECT

public:
    ContainmentInterface(Latte::View *parent);
    virtual ~ContainmentInterface();

    bool applicationLauncherInPopup() const;
    bool applicationLauncherHasGlobalShortcut() const;
    bool containsApplicationLauncher() const;
    bool isCapableToShowShortcutBadges();

    bool activateEntry(const int index);
    bool newInstanceForEntry(const int index);

    bool activatePlasmaTask(const int index);
    bool newInstanceForPlasmaTask(const int index);

    bool hideShortcutBadges();
    bool showOnlyMeta();
    bool showShortcutBadges(const bool showLatteShortcuts, const bool showMeta);

    //! this is updated from external apps e.g. a thunderbird plugin
    bool updateBadgeForLatteTask(const QString identifier, const QString value);

    int applicationLauncherId() const;
    int appletIdForIndex(const int index);

private slots:
    void identifyMainItem();
    void identifyMethods();

private:
    QMetaMethod m_activateEntryMethod;
    QMetaMethod m_appletIdForIndexMethod;
    QMetaMethod m_newInstanceMethod;
    QMetaMethod m_showShortcutsMethod;

    QPointer<Latte::Corona> m_corona;
    QPointer<Latte::View> m_view;
    QPointer<QQuickItem> m_mainItem;
};

}
}

#endif
