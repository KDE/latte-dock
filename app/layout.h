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

#ifndef LAYOUT_H
#define LAYOUT_H

#include <QObject>
#include <QScreen>

#include <Plasma>

#include <KConfigGroup>
#include <KSharedConfig>

namespace Plasma {
class Applet;
class Containment;
class Types;
}

namespace Latte {
class DockCorona;
class DockView;
}

namespace Latte {

//! This class is responsible to hold the settings for a specific layout.
//! It also updates always the relevant layout configuration concerning
//! its general settings (no the containments)
class Layout : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showInMenu READ showInMenu WRITE setShowInMenu NOTIFY showInMenuChanged)
    Q_PROPERTY(QString background READ background NOTIFY backgroundChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString lastUsedActivity READ lastUsedActivity NOTIFY lastUsedActivityChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY textColorChanged)
    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)
    Q_PROPERTY(QStringList activities READ activities WRITE setActivities NOTIFY activitiesChanged)

public:
    Layout(QObject *parent, QString layoutFile, QString layoutName = QString());
    ~Layout() override;

    static const QString MultipleLayoutsName;

    void initToCorona(DockCorona *corona);
    void syncToLayoutFile(bool removeLayoutId = false);
    void unloadContainments();
    void unloadDockViews();

    bool disableBordersForMaximizedWindows() const;
    void setDisableBordersForMaximizedWindows(bool disable);

    bool showInMenu() const;
    void setShowInMenu(bool show);

    bool layoutIsBroken() const;

    //!this layout is loaded and running
    bool isActiveLayout() const;
    //!it is original layout compared to pseudo-layouts that are combinations of multiple-original layouts
    bool isOriginalLayout() const;

    bool isWritable() const;

    int version() const;
    void setVersion(int ver);

    QString background() const;
    void setBackground(QString path);

    QString color() const;
    void setColor(QString color);

    QString lastUsedActivity();
    void clearLastUsedActivity(); //!e.g. when we export a layout

    QString name() const;
    QString file() const;

    QString textColor() const;
    void setTextColor(QString color);

    QStringList activities() const;
    void setActivities(QStringList activities);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    static QString layoutName(const QString &fileName);

    void renameLayout(QString newName);

    QStringList unloadedContainmentsIds();

    //! this function needs the layout to have first set the corona through initToCorona() function
    void addDock(Plasma::Containment *containment, bool forceLoading = false, int expDockScreen = -1);
    void copyDock(Plasma::Containment *containment);
    void recreateDock(Plasma::Containment *containment);

    void syncDockViewsToScreens();
    void importToCorona();

    const QStringList appliedActivities();

    QList<Plasma::Containment *> *containments();
    QHash<const Plasma::Containment *, DockView *> *dockViews();

    //! Bind this dockView and its relevant containments(including systrays)
    //! to this layout. It is used for moving a dockView from layout to layout)
    void assignToLayout(DockView *dockView, QList<Plasma::Containment *> containments);

    //! Unassign that dockView from this layout (this is used for moving a dockView
    //! from layout to layout) and returns all the containments relevant to
    //! that dockView
    QList<Plasma::Containment *> unassignFromLayout(DockView *dockView);

    QList<Plasma::Types::Location> freeEdges(QScreen *screen) const;
    QList<Plasma::Types::Location> freeEdges(int screen) const;

    //! make it only read-only
    void lock();
    //! make it writable which it should be the default
    void unlock();

    int noDocksWithTasks() const;
    int docksCount() const;
    int docksCount(int screen) const;
    int docksCount(QScreen *screen) const;

signals:
    void activitiesChanged();
    void backgroundChanged();
    void colorChanged();
    void disableBordersForMaximizedWindowsChanged();
    void dockColorizerSupportChanged();
    void fileChanged();
    void lastUsedActivityChanged();
    void launchersChanged();
    void nameChanged();
    void versionChanged();
    void showInMenuChanged();
    void textColorChanged();

private slots:
    void loadConfig();
    void saveConfig();

    void addContainment(Plasma::Containment *containment);
    void appletCreated(Plasma::Applet *applet);
    void destroyedChanged(bool destroyed);
    void containmentDestroyed(QObject *cont);
    void updateLastUsedActivity();

private:
    void importLocalLayout(QString file);
    void init();
    void setName(QString name);
    void setFile(QString file);

    bool explicitDockOccupyEdge(int screen, Plasma::Types::Location location) const;

    bool kwin_disabledMaximizedBorders() const;
    void kwin_setDisabledMaximizedBorders(bool disable);

    QString availableId(QStringList all, QStringList assigned, int base);
    //! provides a new file path based the provided file. The new file
    //! has updated ids for containments and applets based on the corona
    //! loaded ones
    QString newUniqueIdsLayoutFromFile(QString file);
    //! imports a layout file and returns the containments for the docks
    QList<Plasma::Containment *> importLayoutFile(QString file);

private:
    bool m_disableBordersForMaximizedWindows{false};
    bool m_showInMenu{false};
    //if version doesnt exist it is and old layout file
    int m_version{2};

    QString m_background;
    QString m_color;
    QString m_lastUsedActivity; //the last used activity for this layout
    QString m_layoutFile;
    QString m_layoutName;
    QString m_textColor;
    QStringList m_activities;
    QStringList m_launchers;

    QStringList m_unloadedContainmentsIds;

    DockCorona *m_corona{nullptr};
    KConfigGroup m_layoutGroup;

    QList<Plasma::Containment *> m_containments;

    QHash<const Plasma::Containment *, DockView *> m_dockViews;
    QHash<const Plasma::Containment *, DockView *> m_waitingDockViews;
};

}

#endif // LAYOUT_H
