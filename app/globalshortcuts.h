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
#include <QTimer>

// KDE
#include <kmodifierkeyinfo.h>

namespace Plasma {
class Containment;
}

namespace Latte {
class Corona;
class View;
}

namespace Latte {

class GlobalShortcuts : public QObject
{
    Q_OBJECT

public:
    GlobalShortcuts(QObject *parent = nullptr);
    ~GlobalShortcuts() override;

    void activateLauncherMenu();
    void updateDockItemBadge(QString identifier, QString value);

signals:
    void modifiersChanged();

private slots:
    void hideDocksTimerSlot();

private:
    void init();
    void initModifiers();
    void activateEntry(int index, Qt::Key modifier);
    void showDocks();
    void showSettings();

    bool activateLatteEntryAtContainment(const Latte::View *view, int index, Qt::Key modifier);
    bool activatePlasmaTaskManagerEntryAtContainment(const Plasma::Containment *c, int index, Qt::Key modifier);
    bool dockAtLowerEdgePriority(Latte::View *test, Latte::View *base);
    bool dockAtLowerScreenPriority(Latte::View *test, Latte::View *base);
    bool docksToHideAreValid();
    bool isCapableToShowAppletsNumbers(Latte::View *view);

    int applicationLauncherId(const Plasma::Containment *c);

    //! <key> modifier is tracked for changes
    bool modifierIsTracked(Qt::Key key);

    //! none of tracked modifiers is pressed
    bool noModifierPressed();

    //! at least one of the modifiers from KeySequence is pressed
    bool sequenceModifierPressed(const QKeySequence &seq);

    //! only <key> is pressed and no other modifier
    bool singleModifierPressed(Qt::Key key);

    //! adjust key in more general values, e.g. Super_L and Super_R both return Super_L
    Qt::Key normalizeKey(Qt::Key key);

    QList<Latte::View *> sortedViewsList(QHash<const Plasma::Containment *, Latte::View *> *views);

    QAction *m_lastInvokedAction;
    //!it is used when the dock is hidden in order to delay the app launcher showing
    QAction *m_singleMetaAction;

    QTimer m_hideDocksTimer;
    QList<Latte::View *> m_hideDocks;

    QList<QQuickItem *> m_calledItems;
    QList<QMetaMethod> m_methodsShowNumbers;

    Latte::Corona *m_corona{nullptr};

    KModifierKeyInfo m_keyInfo;
    QTimer m_metaPressedTimer;

    //! keep a record for modifiers
    QHash<Qt::Key, bool> m_pressed;
};

}

#endif // GLOBALSHORTCUTS_H
