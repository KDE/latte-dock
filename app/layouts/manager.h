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

#ifndef LAYOUTSMANAGER_H
#define LAYOUTSMANAGER_H

// local
#include "launcherssignals.h"
#include "synchronizer.h"
#include "settings/settingsdialog.h"

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
class SharedLayout;
class View;
namespace Layout {
class GenericLayout;
}
namespace Layouts {
class Importer;
class LaunchersSignals;
class Synchronizer;
}
}

namespace Latte {
namespace Layouts {

//! This class is responsible to manipulate all layouts.
//! add,remove,rename, update configurations etc.
class Manager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList layouts READ layouts NOTIFY layoutsChanged)
    Q_PROPERTY(QStringList menuLayouts READ menuLayouts NOTIFY menuLayoutsChanged)

    Q_PROPERTY(LaunchersSignals *launchersSignals READ launchersSignals NOTIFY launchersSignalsChanged)

public:
    Manager(QObject *parent = nullptr);
    ~Manager() override;

    Latte::Corona *corona();
    Importer *importer();

    void load();
    void loadLayoutOnStartup(QString layoutName);
    void showInfoWindow(QString info, int duration, QStringList activities = {"0"});
    void unload();

    void hideAllViews();
    void pauseLayout(QString layoutName);
    void syncLatteViewsToScreens();
    void syncActiveLayoutsToOriginalFiles();

    bool latteViewExists(Latte::View *view) const;
    bool layoutExists(QString layoutName) const;

    QString shouldSwitchToLayout(QString activityId);

    QString currentLayoutName() const;
    QString defaultLayoutName() const;

    QStringList layouts() const;
    QStringList menuLayouts() const;
    QStringList presetsPaths() const;
    QStringList storedSharedLayouts() const;

    Types::LayoutsMemoryUsage memoryUsage() const;
    void setMemoryUsage(Types::LayoutsMemoryUsage memoryUsage);

    //! returns an central layout with that #id (name), it returns null if such
    //! layout cant be found
    CentralLayout *centralLayout(QString id) const;
    int centralLayoutPos(QString id) const;
    SharedLayout *sharedLayout(QString id) const;
    //! return an central or shared layout with #id (name), it returns null if such
    //! loaded layout was not found
    Layout::GenericLayout *layout(QString id) const;

    //! returns the current and central layout based on activities and user preferences
    CentralLayout *currentLayout() const;
    LaunchersSignals *launchersSignals() const;
    Synchronizer *synchronizer() const;

    QStringList activities();
    QStringList runningActivities();
    QStringList orphanedActivities(); //! These are activities that haven't been assigned to specific layout

    void importDefaultLayout(bool newInstanceIfPresent = false);
    void importPresets(bool includeDefault = false);

    bool registerAtSharedLayout(CentralLayout *central, QString id);

public slots:
    void showAboutDialog();

    void hideLatteSettingsDialog();
    Q_INVOKABLE void showLatteSettingsDialog(int page = Latte::Types::LayoutPage);

    //! switch to specified layout, default previousMemoryUsage means that it didn't change
    Q_INVOKABLE bool switchToLayout(QString layoutName, int previousMemoryUsage = -1);

    Q_INVOKABLE int layoutsMemoryUsage();

    //! creates a new layout with layoutName based on the preset
    Q_INVOKABLE QString newLayout(QString layoutName, QString preset = i18n("Default"));

    Q_INVOKABLE QStringList centralLayoutsNames();
    Q_INVOKABLE QStringList sharedLayoutsNames();

signals:
    void centralLayoutsChanged();
    void currentLayoutChanged();
    void currentLayoutNameChanged();
    void launchersSignalsChanged();
    void layoutsChanged();
    void menuLayoutsChanged();

    void currentLayoutIsSwitching(QString layoutName);

private:
    void cleanupOnStartup(QString path); //!remove deprecated or oldstyle config options
    void clearUnloadedContainmentsFromLinkedFile(QStringList containmentsIds, bool bypassChecks = false);

    //! it is used just in order to provide translations for the presets
    void ghostForTranslatedPresets();

    void importPreset(int presetNo, bool newInstanceIfPresent = false);
    void loadLatteLayout(QString layoutPath);

    void setMenuLayouts(QStringList layouts);

private:
    QStringList m_presetsPaths;

    QPointer<Latte::SettingsDialog> m_latteSettingsDialog;

    Latte::Corona *m_corona{nullptr};
    Importer *m_importer{nullptr};
    LaunchersSignals *m_launchersSignals{nullptr};
    Synchronizer *m_synchronizer{nullptr};

    friend class Latte::SettingsDialog;
    friend class Synchronizer;
};

}
}

#endif // LAYOUTSMANAGER_H
