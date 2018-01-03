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

#ifndef LAYOUTMANAGER_H
#define LAYOUTMANAGER_H

#include "dockcorona.h"
#include "importer.h"
#include "layoutsettings.h"
#include "layoutconfigdialog.h"
#include "launcherssignals.h"

#include <QAction>
#include <QObject>

#include <KLocalizedString>

class Importer;
class LayoutSettings;
class LayoutConfigDialog;
class LaunchersSignals;

namespace Latte {

//! This class is responsible to manipulate all layouts.
//! add,remove,rename, update configurations etc.
class LayoutManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList layouts READ layouts NOTIFY layoutsChanged)
    Q_PROPERTY(QStringList menuLayouts READ menuLayouts NOTIFY menuLayoutsChanged)

    Q_PROPERTY(QAction *addWidgetsAction READ addWidgetsAction NOTIFY addWidgetsActionChanged)

    Q_PROPERTY(LayoutSettings *currentLayout READ currentLayout NOTIFY currentLayoutChanged)
    Q_PROPERTY(LaunchersSignals *launchersSignals READ launchersSignals NOTIFY launchersSignalsChanged)

public:
    LayoutManager(QObject *parent = nullptr);
    ~LayoutManager() override;

    DockCorona *corona();
    Importer *importer();

    void load();

    bool layoutExists(QString layoutName) const;

    QString shouldSwitchToLayout(QString activityId);

    QString currentLayoutName() const;
    QString defaultLayoutName() const;

    QStringList layouts() const;
    QStringList menuLayouts() const;
    QStringList presetsPaths() const;

    QAction *addWidgetsAction();

    LayoutSettings *currentLayout();
    LaunchersSignals *launchersSignals();

    QStringList activities();

    void importDefaultLayout(bool newInstanceIfPresent = false);
    void importPresets(bool includeDefault = false);

public slots:
    //! switch to specified layout
    Q_INVOKABLE bool switchToLayout(QString layoutName);

    //! creates a new layout with layoutName based on the preset
    Q_INVOKABLE QString newLayout(QString layoutName, QString preset = i18n("Default"));

    Q_INVOKABLE void showLayoutConfigDialog();

signals:
    void addWidgetsActionChanged();
    void currentLayoutChanged();
    void currentLayoutNameChanged();
    void currentLayoutIsChanging();
    void launchersSignalsChanged();
    void layoutsChanged();
    void menuLayoutsChanged();

private slots:
    void currentActivityChanged(const QString &id);
    void showInfoWindowChanged();
    void showWidgetsExplorer();

private:
    void confirmDynamicSwitch();
    void setMenuLayouts(QStringList layouts);
    void showInfoWindow(QString info, int duration);
    void importPreset(int presetNo, bool newInstanceIfPresent = false);

    QString layoutPath(QString layoutName);

    void loadLayouts();

private:
    DockCorona *m_corona{nullptr};
    Importer *m_importer{nullptr};

    LayoutSettings *m_currentLayout{nullptr};
    LaunchersSignals *m_launchersSignals{nullptr};

    QString m_shouldSwitchToLayout;

    //! it is used just in order to provide translations for the presets
    void ghostForTranslatedPresets();

    bool layoutIsAssigned(QString layoutName);
    QStringList validActivities(QStringList currentList);

    QStringList m_layouts;
    QStringList m_menuLayouts;
    QStringList m_presetsPaths;

    QAction *m_addWidgetsAction{nullptr};

    QPointer<LayoutConfigDialog> m_layoutConfigDialog;

    QHash<const QString, QString> m_assignedLayouts;

    QTimer m_dynamicSwitchTimer;

    friend class LayoutConfigDialog;
};

}

#endif // LAYOUTMANAGER_H
