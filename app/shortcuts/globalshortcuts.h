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

#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

// local
#include "../liblatte2/types.h"

// Qt
#include <QMetaMethod>
#include <QQuickItem>
#include <QPointer>
#include <QTimer>

// KDE
#include <kmodifierkeyinfo.h>

namespace Plasma {
class Containment;
}

namespace Latte {
class Corona;
class View;
namespace ShortcutsPart{
class ModifierTracker;
class ShortcutsTracker;
}
}

namespace Latte {

class GlobalShortcuts : public QObject
{
    Q_OBJECT

public:
    GlobalShortcuts(QObject *parent = nullptr);
    ~GlobalShortcuts() override;

    void activateLauncherMenu();
    void updateViewItemBadge(QString identifier, QString value);

    ShortcutsPart::ShortcutsTracker *shortcutsTracker() const;

    Latte::View *highestPriorityView();

signals:
    void modifiersChanged();

private slots:
    void hideViewsTimerSlot();

private:
    void init();
    void initModifiers();
    void activateEntry(int index, Qt::Key modifier);
    void showViews();
    void showSettings();

    bool activateLatteEntryAtContainment(const Latte::View *view, int index, Qt::Key modifier);
    bool activatePlasmaTaskManagerEntryAtContainment(const Plasma::Containment *c, int index, Qt::Key modifier);
    bool viewAtLowerEdgePriority(Latte::View *test, Latte::View *base);
    bool viewAtLowerScreenPriority(Latte::View *test, Latte::View *base);
    bool viewsToHideAreValid();
    bool isCapableToShowShortcutBadges(Latte::View *view);

    int applicationLauncherId(const Plasma::Containment *c);
    QList<Latte::View *> sortedViewsList(QHash<const Plasma::Containment *, Latte::View *> *views);

private:
    bool m_metaShowedViews{false};

    //! last action that was trigerred from the user
    QAction *m_lastInvokedAction;
    //! it is used for code compatibility reasons in order to replicate a single Meta action
    QAction *m_singleMetaAction;

    //! delayer for hiding the shown latte views
    QTimer m_hideViewsTimer;
    QList<Latte::View *> m_hideViews;

    QList<QQuickItem *> m_viewItemsCalled;
    QList<QMetaMethod> m_showShortcutBadgesMethods;

    QPointer<ShortcutsPart::ModifierTracker> m_modifierTracker;
    QPointer<ShortcutsPart::ShortcutsTracker> m_shortcutsTracker;
    QPointer<Latte::Corona> m_corona;
};

}

#endif // GLOBALSHORTCUTS_H
