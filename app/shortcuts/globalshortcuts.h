/*
    SPDX-FileCopyrightText: 2016 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2016 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GLOBALSHORTCUTS_H
#define GLOBALSHORTCUTS_H

// local
#include <coretypes.h>

// Qt
#include <QAction>
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
    static constexpr const char* SHORTCUTBLOCKHIDINGTYPE  = "globalshortcuts::blockHiding()";

    GlobalShortcuts(QObject *parent = nullptr);
    ~GlobalShortcuts() override;

    void activateLauncherMenu();
    void updateViewItemBadge(QString identifier, QString value);

    ShortcutsPart::ShortcutsTracker *shortcutsTracker() const;

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

    bool activateLatteEntry(Latte::View *view, int index, Qt::Key modifier, bool *delayedExecution);
    bool activatePlasmaTaskManager(const Latte::View *view, int index, Qt::Key modifier, bool *delayedExecution);
    bool activateEntryForView(Latte::View *view, int index, Qt::Key modifier);
    bool viewAtLowerEdgePriority(Latte::View *test, Latte::View *base);
    bool viewAtLowerScreenPriority(Latte::View *test, Latte::View *base);
    bool viewsToHideAreValid();

    //! highest priority application launcher view
    Latte::View *highestApplicationLauncherView(const QList<Latte::View *> &views) const;

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

    QPointer<ShortcutsPart::ModifierTracker> m_modifierTracker;
    QPointer<ShortcutsPart::ShortcutsTracker> m_shortcutsTracker;
    QPointer<Latte::Corona> m_corona;
};

}

#endif // GLOBALSHORTCUTS_H
