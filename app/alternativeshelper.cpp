/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "alternativeshelper.h"

// Qt
#include <QQmlEngine>
#include <QQmlContext>

// KDE
#include <KPackage/Package>
#include <kconfig_version.h>

// Plasma
#include <Plasma/Containment>
#include <Plasma/PluginLoader>

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

