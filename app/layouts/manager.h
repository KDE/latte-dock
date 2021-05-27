/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTSMANAGER_H
#define LAYOUTSMANAGER_H

// local
#include "syncedlaunchers.h"
#include "synchronizer.h"
#include "../apptypes.h"
#include "../data/layoutdata.h"
#include "../data/layouticondata.h"
#include "../settings/settingsdialog/settingsdialog.h"

// Qt
#include <QAction>
#include <QObject>
#include <QPointer>

// KDE
#include <KLocalizedString>

namespace Plasma {
class Containment;
class Types;
}

namespace Latte {
class Corona;
class CentralLayout;
namespace Layouts {
class Importer;
class SyncedLaunchers;
class Synchronizer;
}
}

namespace Latte {
namespace Layouts {

//! Layouts::Manager is a very IMPORTANT class which is responsible to
//! to provide the qml accessible Layouts manipulation API and at the
//! same time to interact with Latte::Corona in order
//! to update correctly the underlying Layouts files by using also
//! its Importer object
//!
//! This class is responsible both for ACTIVE/PASSIVE Layouts.
//!
//! ACTIVE Layout is consider one layout that is loaded and active in memory
//! PASSIVE Layouts is consider one layout that is not loaded/active in memory
//! and its properties are just stored in the filesystem
//!

class Manager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SyncedLaunchers *syncedLaunchers READ syncedLaunchers NOTIFY syncedLaunchersChanged)

public:
    Manager(QObject *parent = nullptr);
    ~Manager() override;

    Latte::Corona *corona();
    Importer *importer();

    void init();
    void loadLayoutOnStartup(QString layoutName);
    void setOnAllActivities(QString layoutName);
    void setOnActivities(QString layoutName, QStringList activities);
    void showInfoWindow(QString info, int duration, QStringList activities = {"0"});
    void unload();

    QStringList currentLayoutsNames() const;

    Latte::Data::LayoutIcon iconForLayout(const QString &storedLayoutName) const;
    Latte::Data::LayoutIcon iconForLayout(const Data::Layout &layout) const;

    MemoryUsage::LayoutsMemory memoryUsage() const;
    void setMemoryUsage(MemoryUsage::LayoutsMemory memoryUsage);

    //! switch to specified layout, default previousMemoryUsage means that it didn't change
    bool switchToLayout(QString layoutName,  MemoryUsage::LayoutsMemory newMemoryUsage = MemoryUsage::Current);

    //! returns the current and central layout based on activities and user preferences
    QList<CentralLayout *>currentLayouts() const;
    SyncedLaunchers *syncedLaunchers() const;
    Synchronizer *synchronizer() const;

    void moveView(QString originLayoutName, uint originViewId, QString destinationLayoutName);

public slots:
    void showAboutDialog();

    void hideLatteSettingsDialog();
    Q_INVOKABLE void showLatteSettingsDialog(int firstPage = Settings::Dialog::LayoutPage, bool toggleCurrentPage = false);
    Q_INVOKABLE QStringList centralLayoutsNames();
    Q_INVOKABLE QStringList viewTemplateNames() const;
    Q_INVOKABLE QStringList viewTemplateIds() const;

signals:
    void centralLayoutsChanged();
    void syncedLaunchersChanged();
    void viewTemplatesChanged();

    void currentLayoutIsSwitching(QString layoutName);

    //! used from ConfigView(s) in order to be informed which is one should be shown
    void lastConfigViewChangedFrom(Latte::View *view);

private:
    void cleanupOnStartup(QString path); //!remove deprecated or oldstyle config options
    void clearUnloadedContainmentsFromLinkedFile(QStringList containmentsIds, bool bypassChecks = false);

    void loadLatteLayout(QString layoutPath);

    void setMenuLayouts(QStringList layouts);

private:
    QPointer<Latte::Settings::Dialog::SettingsDialog> m_latteSettingsDialog;

    Latte::Corona *m_corona{nullptr};
    Importer *m_importer{nullptr};
    SyncedLaunchers *m_syncedLaunchers{nullptr};
    Synchronizer *m_synchronizer{nullptr};

    friend class Latte::Settings::Dialog::SettingsDialog;
    friend class Synchronizer;
};

}
}

#endif // LAYOUTSMANAGER_H
