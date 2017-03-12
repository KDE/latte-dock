/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "alternativeshelper.h"

#include <QQmlEngine>
#include <QQmlContext>

#include <Plasma/Containment>
#include <Plasma/PluginLoader>
#include <KPackage/Package>
#include <kconfig_version.h>

AlternativesHelper::AlternativesHelper(Plasma::Applet *applet, QObject *parent)
    : QObject(parent),
      m_applet(applet)
{
}

AlternativesHelper::~AlternativesHelper()
{
}

QStringList AlternativesHelper::appletProvides() const
{
#if KCONFIG_VERSION_MINOR >= 27
    return KPluginMetaData::readStringList(m_applet->pluginMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));
#else
    return KPluginMetaData::readStringList(m_applet->pluginInfo().toMetaData().rawData(), QStringLiteral("X-Plasma-Provides"));
#endif
}

QString AlternativesHelper::currentPlugin() const
{
#if KCONFIG_VERSION_MINOR >= 27
    return m_applet->pluginMetaData().pluginId();
#else
    return m_applet->pluginInfo().toMetaData().pluginId();
#endif
}

QQuickItem *AlternativesHelper::applet() const
{
    return m_applet->property("_plasma_graphicObject").value<QQuickItem *>();
}

void AlternativesHelper::loadAlternative(const QString &plugin)
{
    if (plugin == currentPlugin() || m_applet->isContainment()) {
        return;
    }

    Plasma::Containment *cont = m_applet->containment();

    if (!cont) {
        return;
    }

    QQuickItem *appletItem = m_applet->property("_plasma_graphicObject").value<QQuickItem *>();
    QQuickItem *contItem = cont->property("_plasma_graphicObject").value<QQuickItem *>();

    if (!appletItem || !contItem) {
        return;
    }

    // ensure the global shortcut is moved to the new applet
    const QKeySequence &shortcut = m_applet->globalShortcut();
    m_applet->setGlobalShortcut(QKeySequence()); // need to unmap the old one first

    const QPoint newPos = appletItem->mapToItem(contItem, QPointF(0, 0)).toPoint();

    m_applet->destroy();

    connect(m_applet, &QObject::destroyed, [ = ]() {
        Plasma::Applet *newApplet = Q_NULLPTR;
        QMetaObject::invokeMethod(contItem, "createApplet", Q_RETURN_ARG(Plasma::Applet *, newApplet), Q_ARG(QString, plugin), Q_ARG(QVariantList, QVariantList()), Q_ARG(QPoint, newPos));

        if (newApplet) {
            newApplet->setGlobalShortcut(shortcut);
        }
    });
}

#include "moc_alternativeshelper.cpp"

