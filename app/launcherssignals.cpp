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

#include "launcherssignals.h"

namespace Latte {

LaunchersSignals::LaunchersSignals(QObject *parent)
    : QObject(parent)
{
    m_manager = qobject_cast<LayoutManager *>(parent);
}

LaunchersSignals::~LaunchersSignals()
{
}

QList<Plasma::Applet *> LaunchersSignals::lattePlasmoids()
{
    QList<Plasma::Applet *> applets;

    foreach (auto containment, m_manager->corona()->containments()) {
        for (auto *applet : containment->applets()) {
            KPluginMetaData meta = applet->kPackage().metadata();

            if (meta.pluginId() == "org.kde.latte.plasmoid") {
                applets.append(applet);
            }
        }
    }

    return applets;
}

void LaunchersSignals::addLauncher(int launcherGroup, QString launcher)
{
    if ((Dock::LaunchersGroup)launcherGroup == Dock::UniqueLaunchers) {
        return;
    }

    foreach (auto applet, lattePlasmoids()) {
        if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = appletInterface->childItems();

            if (childItems.isEmpty()) {
                continue;
            }

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    int methodIndex = metaObject->indexOfMethod("extSignalAddLauncher(QVariant,QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    QMetaMethod method = metaObject->method(methodIndex);
                    method.invoke(item, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher));
                }
            }
        }
    }
}

void LaunchersSignals::removeLauncher(int launcherGroup, QString launcher)
{
    if ((Dock::LaunchersGroup)launcherGroup == Dock::UniqueLaunchers) {
        return;
    }

    foreach (auto applet, lattePlasmoids()) {
        if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = appletInterface->childItems();

            if (childItems.isEmpty()) {
                continue;
            }

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    int methodIndex = metaObject->indexOfMethod("extSignalRemoveLauncher(QVariant,QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    QMetaMethod method = metaObject->method(methodIndex);
                    method.invoke(item, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher));
                }
            }
        }
    }
}

void LaunchersSignals::addLauncherToActivity(int launcherGroup, QString launcher, QString activity)
{
    if ((Dock::LaunchersGroup)launcherGroup == Dock::UniqueLaunchers) {
        return;
    }

    foreach (auto applet, lattePlasmoids()) {
        if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = appletInterface->childItems();

            if (childItems.isEmpty()) {
                continue;
            }

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    int methodIndex = metaObject->indexOfMethod("extSignalAddLauncherToActivity(QVariant,QVariant,QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    QMetaMethod method = metaObject->method(methodIndex);
                    method.invoke(item, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher), Q_ARG(QVariant, activity));
                }
            }
        }
    }
}

void LaunchersSignals::removeLauncherFromActivity(int launcherGroup, QString launcher, QString activity)
{
    if ((Dock::LaunchersGroup)launcherGroup == Dock::UniqueLaunchers) {
        return;
    }

    foreach (auto applet, lattePlasmoids()) {
        if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = appletInterface->childItems();

            if (childItems.isEmpty()) {
                continue;
            }

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    int methodIndex = metaObject->indexOfMethod("extSignalRemoveLauncherFromActivity(QVariant,QVariant,QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    QMetaMethod method = metaObject->method(methodIndex);
                    method.invoke(item, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, launcher), Q_ARG(QVariant, activity));
                }
            }
        }
    }
}

void LaunchersSignals::urlsDropped(int launcherGroup, QStringList urls)
{
    if ((Dock::LaunchersGroup)launcherGroup == Dock::UniqueLaunchers) {
        return;
    }

    foreach (auto applet, lattePlasmoids()) {
        if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = appletInterface->childItems();

            if (childItems.isEmpty()) {
                continue;
            }

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    int methodIndex = metaObject->indexOfMethod("extSignalUrlsDropped(QVariant,QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    QMetaMethod method = metaObject->method(methodIndex);
                    method.invoke(item, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, urls));
                }
            }
        }
    }
}

void LaunchersSignals::moveTask(int launcherGroup, int from, int to)
{
    if ((Dock::LaunchersGroup)launcherGroup == Dock::UniqueLaunchers) {
        return;
    }

    foreach (auto applet, lattePlasmoids()) {
        if (QQuickItem *appletInterface = applet->property("_plasma_graphicObject").value<QQuickItem *>()) {
            const auto &childItems = appletInterface->childItems();

            if (childItems.isEmpty()) {
                continue;
            }

            for (QQuickItem *item : childItems) {
                if (auto *metaObject = item->metaObject()) {
                    int methodIndex = metaObject->indexOfMethod("extSignalMoveTask(QVariant,QVariant,QVariant)");

                    if (methodIndex == -1) {
                        continue;
                    }

                    QMetaMethod method = metaObject->method(methodIndex);
                    method.invoke(item, Q_ARG(QVariant, launcherGroup), Q_ARG(QVariant, from), Q_ARG(QVariant, to));
                }
            }
        }
    }
}



} //end of namespace
