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

#include "dockcorona.h"
#include "dockview.h"
#include "packageplugins/shell/dockpackage.h"
#include "../liblattedock/windowsystem.h"
#include "alternativeshelper.h"

#include <QAction>
#include <QScreen>
#include <QDebug>
#include <QQmlContext>

#include <Plasma>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <KActionCollection>
#include <KPluginMetaData>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KAboutData>
#include <KActivities/Consumer>

namespace Latte {

DockCorona::DockCorona(QStringList debugFlags, QObject *parent)
    : Plasma::Corona(parent),
      m_debugFlags(debugFlags),
      m_activityConsumer(new KActivities::Consumer(this))
{
    KPackage::Package package(new DockPackage(this));

    if (!package.isValid()) {
        qWarning() << staticMetaObject.className()
                   << "the package" << package.metadata().rawData() << "is invalid!";
        return;
    } else {
        qDebug() << staticMetaObject.className()
                 << "the package" << package.metadata().rawData() << "is valid!";
    }

    setKPackage(package);
    qmlRegisterTypes();
    connect(this, &Corona::containmentAdded, this, &DockCorona::addDock);

    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running)) {
        load();
    }

    connect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
}

DockCorona::~DockCorona()
{
    cleanConfig();

    while (!containments().isEmpty()) {
        //deleting a containment will remove it from the list due to QObject::destroyed connect in Corona
        delete containments().first();
    }

    qDeleteAll(m_dockViews);
    qDeleteAll(m_waitingDockViews);
    m_dockViews.clear();
    m_waitingDockViews.clear();
    disconnect(m_activityConsumer, &KActivities::Consumer::serviceStatusChanged, this, &DockCorona::load);
    delete m_activityConsumer;
    qDebug() << "deleted" << this;
}

void DockCorona::load()
{
    if (m_activityConsumer && (m_activityConsumer->serviceStatus() == KActivities::Consumer::Running) && m_activitiesStarting) {
        m_activitiesStarting = false;
        loadLayout();
    }
}

void DockCorona::cleanConfig()
{
    auto containmentsEntries = config()->group("Containments");
    bool changed = false;

    foreach (auto cId, containmentsEntries.groupList()) {
        if (!containmentExists(cId.toUInt())) {
            //cleanup obsolete containments
            containmentsEntries.group(cId).deleteGroup();
            changed = true;
            qDebug() << "obsolete containment configuration deleted:" << cId;
        } else {
            //cleanup obsolete applets of running containments
            auto appletsEntries = containmentsEntries.group(cId).group("Applets");

            foreach (auto appletId, appletsEntries.groupList()) {
                if (!appletExists(cId.toUInt(), appletId.toUInt())) {
                    appletsEntries.group(appletId).deleteGroup();
                    changed = true;
                    qDebug() << "obsolete applet configuration deleted:" << appletId;
                }
            }
        }
    }

    if (changed) {
        config()->sync();
        qDebug() << "configuration file cleaned...";
    }
}

bool DockCorona::containmentExists(uint id) const
{
    foreach (auto containment, containments()) {
        if (id == containment->id()) {
            return true;
        }
    }

    return false;
}

bool DockCorona::appletExists(uint containmentId, uint appletId) const
{
    Plasma::Containment *containment = nullptr;

    foreach (auto cont, containments()) {
        if (containmentId == cont->id()) {
            containment = cont;
            break;
        }
    }

    if (!containment) {
        return false;
    }

    foreach (auto applet, containment->applets()) {
        if (applet->id() == appletId) {
            return true;
        }
    }

    return false;
}

int DockCorona::numScreens() const
{
    return qGuiApp->screens().count();
}

QRect DockCorona::screenGeometry(int id) const
{
    const auto screens = qGuiApp->screens();

    if (id >= 0 && id < screens.count()) {
        return screens[id]->geometry();
    }

    return qGuiApp->primaryScreen()->geometry();
}

QRegion DockCorona::availableScreenRegion(int id) const
{
    return availableScreenRect(id);
    //FIXME::: availableGeometry is probably broken
    // in Qt, so this have to be updated as plasma is doing it
    // for example the availableScreenRect
}

QRect DockCorona::availableScreenRect(int id) const
{
    const auto screens = qGuiApp->screens();
    const QScreen *screen = nullptr;

    if (id >= 0 && id < screens.count())
        screen = screens[id];
    else
        screen = qGuiApp->primaryScreen();

    if (!screen)
        return {};

    auto available = screen->geometry();

    for (const auto *view : m_dockViews) {
        if (view && view->containment() && view->screen() == screen) {
            auto dockRect = view->absGeometry();

            // Usually availableScreenRect is used by the desktop,
            // but Latte dont have desktop, then here just
            // need calculate available space for top and bottom location,
            // because the left and right are those who dodge others docks
            switch (view->location()) {
                case Plasma::Types::TopEdge:
                    available.setTopLeft({available.x(), dockRect.bottom()});
                    break;

                case Plasma::Types::BottomEdge:
                    available.setBottomLeft({available.x(), dockRect.top()});
                    break;
            }
        }
    }

    return available;
}

int DockCorona::primaryScreenId() const
{
    //this is not the proper way because kwin probably uses a different
    //index of screens...
    //This needs a lot of testing...
    return qGuiApp->screens().indexOf(qGuiApp->primaryScreen());
}

int DockCorona::docksCount(int screen) const
{
    if (screen == -1)
        return 0;

    int docks{0};

    for (const auto &view : m_dockViews) {
        if (view && view->containment()
            && view->containment()->screen() == screen
            && !view->containment()->destroyed()) {
            ++docks;
        }
    }

    // qDebug() << docks << "docks on screen:" << screen;
    return docks;
}

void DockCorona::closeApplication()
{
    qGuiApp->quit();
}

QStringList DockCorona::debugFlags() const
{
    return m_debugFlags;
}

void DockCorona::aboutApplication()
{
    if (aboutDialog) {
        aboutDialog->hide();
        aboutDialog->deleteLater();
    }

    aboutDialog = new KAboutApplicationDialog(KAboutData::applicationData());
    connect(aboutDialog.data(), &QDialog::finished, aboutDialog.data(), &QObject::deleteLater);
    WindowSystem::self().skipTaskBar(*aboutDialog);

    aboutDialog->show();
}

QList<Plasma::Types::Location> DockCorona::freeEdges(int screen) const
{
    using Plasma::Types;
    QList<Types::Location> edges{Types::BottomEdge, Types::LeftEdge,
                                 Types::TopEdge, Types::RightEdge};
    //when screen=-1 is passed then the primaryScreenid is used
    int fixedScreen = (screen == -1) ? primaryScreenId() : screen;

    for (auto *view : m_dockViews) {
        if (view && view->containment()
            && view->containment()->screen() == fixedScreen) {
            edges.removeOne(view->location());
        }
    }

    return edges;
}

int DockCorona::screenForContainment(const Plasma::Containment *containment) const
{
    //FIXME: indexOf is not a proper way to support multi-screen
    // as for environment to environment the indexes change
    // also there is the following issue triggered
    // from dockView adaptToScreen()
    //
    // in a multi-screen environment that
    // primary screen is not set to 0 it was
    // created an endless showing loop at
    // startup (catch-up race) between
    // screen:0 and primaryScreen

    return primaryScreenId();
}

void DockCorona::addDock(Plasma::Containment *containment)
{
    if (!containment || !containment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto metadata = containment->kPackage().metadata();

    if (metadata.pluginId() == "org.kde.plasma.private.systemtray") {
        if (metadata.pluginId() != "org.kde.latte.containment")
            return;
    }

    for (auto *dock : m_dockViews) {
        if (dock->containment() == containment)
            return;
    }

    qDebug() << "Adding dock for container...";
    auto dockView = new DockView(this);
    dockView->init();
    dockView->setContainment(containment);
    connect(containment, &QObject::destroyed, this, &DockCorona::dockContainmentDestroyed);
    connect(containment, &Plasma::Applet::destroyedChanged, this, &DockCorona::destroyedChanged);
    connect(containment, &Plasma::Applet::locationChanged, this, &DockCorona::dockLocationChanged);
    connect(containment, &Plasma::Containment::appletAlternativesRequested
            , this, &DockCorona::showAlternativesForApplet, Qt::QueuedConnection);

    dockView->show();
    m_dockViews[containment] = dockView;
    emit docksCountChanged();
}

void DockCorona::destroyedChanged(bool destroyed)
{
    Plasma::Containment *sender = qobject_cast<Plasma::Containment *>(QObject::sender());

    if (!sender) {
        return;
    }

    if (destroyed) {
        m_waitingDockViews[sender] = m_dockViews.take(static_cast<Plasma::Containment *>(sender));
    } else {
        m_dockViews[sender] = m_waitingDockViews.take(static_cast<Plasma::Containment *>(sender));
    }

    emit docksCountChanged();
}

void DockCorona::dockContainmentDestroyed(QObject *cont)
{
    auto view = m_waitingDockViews.take(static_cast<Plasma::Containment *>(cont));

    if (view)
        delete view;

    emit docksCountChanged();
}

void DockCorona::showAlternativesForApplet(Plasma::Applet *applet)
{
    const QString alternativesQML = kPackage().filePath("appletalternativesui");

    if (alternativesQML.isEmpty()) {
        return;
    }

    if (applet->containment()) {
        DockView *view = m_dockViews[applet->containment()];

        if (view) {
            view->visibility()->setBlockHiding(true);
            m_alternativesContaiment = applet->containment();
        }
    }

    KDeclarative::QmlObject *qmlObj = new KDeclarative::QmlObject(this);
    qmlObj->setInitializationDelayed(true);
    qmlObj->setSource(QUrl::fromLocalFile(alternativesQML));

    AlternativesHelper *helper = new AlternativesHelper(applet, qmlObj);
    qmlObj->rootContext()->setContextProperty(QStringLiteral("alternativesHelper"), helper);

    m_alternativesObjects << qmlObj;
    qmlObj->completeInitialization();
    connect(qmlObj->rootObject(), SIGNAL(visibleChanged(bool)),
            this, SLOT(alternativesVisibilityChanged(bool)));

    connect(applet, &Plasma::Applet::destroyedChanged, this, [this, qmlObj](bool destroyed) {
        if (!destroyed) {
            return;
        }

        QMutableListIterator<KDeclarative::QmlObject *> it(m_alternativesObjects);

        while (it.hasNext()) {
            KDeclarative::QmlObject *obj = it.next();

            if (obj == qmlObj) {
                it.remove();
                obj->deleteLater();
            }
        }
    });
}

void DockCorona::alternativesVisibilityChanged(bool visible)
{
    if (visible) {
        return;
    }

    if (m_alternativesContaiment) {
        DockView *view = m_dockViews[m_alternativesContaiment];

        if (view) {
            view->visibility()->setBlockHiding(false);
        }

        m_alternativesContaiment = nullptr;
    }

    QObject *root = sender();

    QMutableListIterator<KDeclarative::QmlObject *> it(m_alternativesObjects);

    while (it.hasNext()) {
        KDeclarative::QmlObject *obj = it.next();

        if (obj->rootObject() == root) {
            it.remove();
            obj->deleteLater();
        }
    }
}

void DockCorona::loadDefaultLayout()
{
    qDebug() << "loading default layout";
    //! Settting mutable for create a containment
    setImmutability(Plasma::Types::Mutable);
    QVariantList args;
    auto defaultContainment = createContainmentDelayed("org.kde.latte.containment", args);
    defaultContainment->setContainmentType(Plasma::Types::PanelContainment);
    defaultContainment->init();

    if (!defaultContainment || !defaultContainment->kPackage().isValid()) {
        qWarning() << "the requested containment plugin can not be located or loaded";
        return;
    }

    auto config = defaultContainment->config();
    defaultContainment->restore(config);
    QList<Plasma::Types::Location> edges = freeEdges(defaultContainment->screen());

    if (edges.count() > 0) {
        defaultContainment->setLocation(edges.at(0));
    } else {
        defaultContainment->setLocation(Plasma::Types::BottomEdge);
    }

    defaultContainment->updateConstraints(Plasma::Types::StartupCompletedConstraint);
    defaultContainment->save(config);
    requestConfigSync();
    defaultContainment->flushPendingConstraintsEvents();
    emit containmentAdded(defaultContainment);
    emit containmentCreated(defaultContainment);
    addDock(defaultContainment);
    defaultContainment->createApplet(QStringLiteral("org.kde.latte.plasmoid"));
    defaultContainment->createApplet(QStringLiteral("org.kde.plasma.analogclock"));
}

inline void DockCorona::qmlRegisterTypes() const
{
    qmlRegisterType<QScreen>();
}

}
