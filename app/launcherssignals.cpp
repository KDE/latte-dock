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

#include "dockcorona.h"
#include "layout.h"

#include <QQuickItem>

#include <Plasma/Applet>
#include <Plasma/Containment>

namespace Latte {

LaunchersSignals::LaunchersSignals(QObject *parent)
    : QObject(parent)
{
    m_manager = qobject_cast<LayoutManager *>(parent);
}

LaunchersSignals::~LaunchersSignals()
{
}

QList<Plasma::Applet *> LaunchersSignals::lattePlasmoids(QString layoutName)
{
    QList<Plasma::Applet *> applets;

    Layout *layout = m_manager->activeLayout(layoutName);
    QList<Plasma::Containment *> containments;

    if (layoutName.isEmpty()) {
        containments = m_manager->corona()->containments();
    } else if (layout) {
        containments = *(layout->containments());
    }

    foreach (auto containment, containments) {
        for (auto *applet : containment->applets()) {
            KPluginMetaData meta = applet->kPackage().metadata();

            if (meta.pluginId() == "org.kde.latte.plasmoid") {
                applets.append(applet);
            }
        }
    }

    return applets;
}

void LaunchersSignals::addLauncher(QString layoutName, int launcherGroup, QString launcher)
{
    Dock::LaunchersGroup group = static_cast<Dock::LaunchersGroup>(launcherGroup);

    if ((Dock::LaunchersGroup)group == Dock::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Dock::LayoutLaunchers) ? layoutName : "";

    foreach (auto applet, lattePlasmoids(lName)) {
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

void LaunchersSignals::removeLauncher(QString layoutName, int launcherGroup, QString launcher)
{
    Dock::LaunchersGroup group = static_cast<Dock::LaunchersGroup>(launcherGroup);

    if ((Dock::LaunchersGroup)group == Dock::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Dock::LayoutLaunchers) ? layoutName : "";

    foreach (auto applet, lattePlasmoids(lName)) {
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

void LaunchersSignals::addLauncherToActivity(QString layoutName, int launcherGroup, QString launcher, QString activity)
{
    Dock::LaunchersGroup group = static_cast<Dock::LaunchersGroup>(launcherGroup);

    if ((Dock::LaunchersGroup)group == Dock::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Dock::LayoutLaunchers) ? layoutName : "";

    foreach (auto applet, lattePlasmoids(lName)) {
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

void LaunchersSignals::removeLauncherFromActivity(QString layoutName, int launcherGroup, QString launcher, QString activity)
{
    Dock::LaunchersGroup group = static_cast<Dock::LaunchersGroup>(launcherGroup);

    if ((Dock::LaunchersGroup)group == Dock::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Dock::LayoutLaunchers) ? layoutName : "";

    foreach (auto applet, lattePlasmoids(lName)) {
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

void LaunchersSignals::urlsDropped(QString layoutName, int launcherGroup, QStringList urls)
{
    Dock::LaunchersGroup group = static_cast<Dock::LaunchersGroup>(launcherGroup);

    if ((Dock::LaunchersGroup)group == Dock::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Dock::LayoutLaunchers) ? layoutName : "";

    foreach (auto applet, lattePlasmoids(lName)) {
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

void LaunchersSignals::moveTask(QString layoutName, int senderId, int launcherGroup, int from, int to)
{
    Dock::LaunchersGroup group = static_cast<Dock::LaunchersGroup>(launcherGroup);

    if ((Dock::LaunchersGroup)group == Dock::UniqueLaunchers) {
        return;
    }

    QString lName = (group == Dock::LayoutLaunchers) ? layoutName : "";

    foreach (auto applet, lattePlasmoids(lName)) {
        if (applet->id() != senderId) {
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
}

} //end of namespace
