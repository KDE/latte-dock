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

#ifndef UNIVERSALSETTINGS_H
#define UNIVERSALSETTINGS_H

// local
#include "../lattecorona.h"
#include "../liblatte2/types.h"

// Qt
#include <QObject>
#include <QAbstractItemModel>
#include <QHash>
#include <QPointer>
#include <QQmlListProperty>
#include <QScreen>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

namespace Latte {

class LayoutManager;

//! This class holds all the settings that are universally available
//! independent of layouts
class UniversalSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(bool showInfoWindow READ showInfoWindow WRITE setShowInfoWindow NOTIFY showInfoWindowChanged)

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName WRITE setCurrentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList badgesForActivate READ badgesForActivate NOTIFY badgesForActivateChanged)
    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)

    Q_PROPERTY(Latte::Types::MouseSensitivity mouseSensitivity READ mouseSensitivity WRITE setMouseSensitivity NOTIFY mouseSensitivityChanged)

    Q_PROPERTY(QQmlListProperty<QScreen> screens READ screens)

public:
    UniversalSettings(KSharedConfig::Ptr config, QObject *parent = nullptr);
    ~UniversalSettings() override;

    void load();

    bool autostart() const;
    void setAutostart(bool state);

    bool canDisableBorders() const;
    void setCanDisableBorders(bool enable);

    bool metaForwardedToLatte() const;
    void forwardMetaToLatte(bool forward);

    bool showInfoWindow() const;
    void setShowInfoWindow(bool show);

    int version() const;
    void setVersion(int ver);

    int screenTrackerInterval() const;
    void setScreenTrackerInterval(int duration);

    QString currentLayoutName() const;
    void setCurrentLayoutName(QString layoutName);

    QString lastNonAssignedLayoutName() const;
    void setLastNonAssignedLayoutName(QString layoutName);

    QSize downloadWindowSize() const;
    void setDownloadWindowSize(QSize size);

    QSize layoutsWindowSize() const;
    void setLayoutsWindowSize(QSize size);

    QStringList badgesForActivate() const;

    QStringList layoutsColumnWidths() const;
    void setLayoutsColumnWidths(QStringList widths);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    Types::MouseSensitivity mouseSensitivity() const;
    void setMouseSensitivity(Types::MouseSensitivity sensitivity);

    QQmlListProperty<QScreen> screens();
    static int countScreens(QQmlListProperty<QScreen> *property); //! is needed by screens()
    static QScreen *atScreens(QQmlListProperty<QScreen> *property, int index); //! is needed by screens()

    void clearAllAppletShortcuts();

public slots:
    Q_INVOKABLE QString splitterIconPath();
    Q_INVOKABLE QString trademarkIconPath();

    Q_INVOKABLE QString appletShortcutBadge(int appletId);

signals:
    void autostartChanged();
    void badgesForActivateChanged();
    void canDisableBordersChanged();
    void currentLayoutNameChanged();
    void downloadWindowSizeChanged();
    void lastNonAssignedLayoutNameChanged();
    void layoutsColumnWidthsChanged();
    void layoutsWindowSizeChanged();
    void launchersChanged();
    void layoutsMemoryUsageChanged();
    void mouseSensitivityChanged();
    void screenTrackerIntervalChanged();
    void showInfoWindowChanged();
    void versionChanged();

private slots:
    void loadConfig();
    void saveConfig();

    void shortcutsFileChanged(const QString &file);

private:
    void cleanupSettings();

    void initGlobalShortcutsWatcher();
    //! access user set global shortcuts for activate entries
    void parseGlobalShortcuts();

    bool kwin_metaForwardedToLatte() const;
    void kwin_forwardMetaToLatte(bool forward);

    Types::LayoutsMemoryUsage layoutsMemoryUsage() const;
    void setLayoutsMemoryUsage(Types::LayoutsMemoryUsage layoutsMemoryUsage);

    QString shortcutToBadge(QStringList shortcutRecords);

private:
    bool m_canDisableBorders{false};
    bool m_showInfoWindow{true};

    //when there isnt a version it is an old universal file
    int m_version{1};

    int m_screenTrackerInterval{2500};

    QString m_currentLayoutName;
    QString m_lastNonAssignedLayoutName;
    QSize m_downloadWindowSize{800, 550};
    QSize m_layoutsWindowSize{700, 450};

    QStringList m_badgesForActivate{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "z", "x", "c", "v", "b", "n", "m", ",", "."};
    QStringList m_layoutsColumnWidths;
    QStringList m_launchers;

    //! shortcuts assigned to applets through plasma infrastructure
    //! <applet id, shortcut>
    QHash<int, QString> m_appletShortcuts;

    Types::LayoutsMemoryUsage m_memoryUsage;
    Types::MouseSensitivity m_mouseSensitivity{Types::HighSensitivity};

    QPointer<Latte::Corona> m_corona;

    KConfigGroup m_universalGroup;
    KSharedConfig::Ptr m_config;
    KSharedConfig::Ptr m_shortcutsConfigPtr;

    friend class LayoutManager;
    friend class Latte::Corona;
};

}

#endif //UNIVERSALSETTINGS_H
