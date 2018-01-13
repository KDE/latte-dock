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

#include <KConfigGroup>
#include <KSharedConfig>

#include "dockcorona.h"

namespace Latte {

class DockCorona;
class DockView;

//! This class is responsible to hold the settings for a specific layout.
//! It also updates always the relevant layout configuration concerning
//! its general settings (no the containments)
class Layout : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool showInMenu READ showInMenu WRITE setShowInMenu NOTIFY showInMenuChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)
    Q_PROPERTY(QStringList activities READ activities WRITE setActivities NOTIFY activitiesChanged)

public:
    Layout(QObject *parent, QString layoutFile, QString layoutName = QString());
    ~Layout() override;

    static const QString MultipleLayoutsName;

    void initToCorona(DockCorona *corona);
    void unloadContainments();
    void unloadDockViews();

    bool showInMenu() const;
    void setShowInMenu(bool show);

    bool fileIsBroken() const;

    int version() const;
    void setVersion(int ver);

    QString name() const;
    QString file() const;

    QString color() const;
    void setColor(QString color);

    QStringList activities() const;
    void setActivities(QStringList activities);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    static QString layoutName(const QString &fileName);

    //! this function needs the layout to have first set the corona through initToCorona() function
    void addDock(Plasma::Containment *containment, bool forceLoading = false, int expDockScreen = -1);
    void copyDock(Plasma::Containment *containment);
    void recreateDock(Plasma::Containment *containment);
    void syncDockViewsToScreens();

    QHash<const Plasma::Containment *, DockView *> *dockViews();

signals:
    void activitiesChanged();
    void colorChanged();
    void fileChanged();
    void launchersChanged();
    void nameChanged();
    void versionChanged();
    void showInMenuChanged();

private slots:
    void loadConfig();
    void saveConfig();

    void addContainment(Plasma::Containment *containment);
    void destroyedChanged(bool destroyed);
    void containmentDestroyed(QObject *cont);

private:
    void importLocalLayout(QString file);
    void init();
    void setName(QString name);
    void setFile(QString file);

    QString availableId(QStringList all, QStringList assigned, int base);
    //! provides a new file path based the provided file. The new file
    //! has updated ids for containments and applets based on the corona
    //! loaded ones
    QString newUniqueIdsLayoutFromFile(QString file);
    //! imports a layout file and returns the containments for the docks
    QList<Plasma::Containment *> importLayoutFile(QString file);

private:
    bool m_showInMenu{false};
    //if version doesnt exist it is and old layout file
    int m_version{2};

    QString m_color;
    QString m_layoutFile;
    QString m_layoutName;
    QStringList m_activities;
    QStringList m_launchers;

    DockCorona *m_corona{nullptr};
    KConfigGroup m_layoutGroup;
    KSharedConfigPtr m_filePtr;

    QList<Plasma::Containment *> m_containments;

    QHash<const Plasma::Containment *, DockView *> m_dockViews;
    QHash<const Plasma::Containment *, DockView *> m_waitingDockViews;
};

}

#endif // LAYOUT_H
