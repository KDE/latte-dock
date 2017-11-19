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

#include "dockcorona.h"
#include "dockview.h"
#include "../liblattedock/dock.h"

#include <QQuickItem>
#include <QMetaMethod>

class DockCorona;
class DockView;

namespace Latte {

class GlobalShortcuts : public QObject {
    Q_OBJECT

public:
    GlobalShortcuts(QObject *parent = nullptr);
    ~GlobalShortcuts() override;

    void activateLauncherMenu();
    void updateDockItemBadge(QString identifier, QString value);

private slots:
    void hideDockTimerSlot();

private:
    void init();
    void activateTaskManagerEntry(int index, Qt::Key modifier);
    void showDock();
    void hideDock();
    void showSettings();

    bool dockAtLowerEdgePriority(DockView *test, DockView *base);
    bool dockAtLowerScreenPriority(DockView *test, DockView *base);

    QAction *m_lastInvokedAction;
    QTimer m_hideDockTimer;
    DockView *m_hideDock;

    int m_tasksMethodIndex{ -1};
    QQuickItem *m_tasksPlasmoid{nullptr};
    QMetaMethod m_methodShowNumbers;

    DockCorona *m_corona{nullptr};
};

}

#endif // GLOBALSHORTCUTS_H
