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
#include <coretypes.h>
#include "../apptypes.h"
#include "../lattecorona.h"

// Qt
#include <QObject>
#include <QAbstractItemModel>
#include <QHash>
#include <QPair>
#include <QPointer>
#include <QQmlListProperty>
#include <QScreen>

// KDE
#include <KConfigGroup>
#include <KSharedConfig>

namespace Latte {
namespace Layouts {
class Manager;
}
}

namespace Latte {
//width_scale, height_scale
typedef QPair<float, float> ScreenScales;

//! This class holds all the settings that are universally available
//! independent of layouts
class UniversalSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(bool badges3DStyle READ badges3DStyle WRITE setBadges3DStyle NOTIFY badges3DStyleChanged)
    Q_PROPERTY(bool colorsScriptIsPresent READ colorsScriptIsPresent NOTIFY colorsScriptIsPresentChanged)
    Q_PROPERTY(bool hiddenConfigurationWindowsAreDeleted READ hiddenConfigurationWindowsAreDeleted WRITE setHiddenConfigurationWindowsAreDeleted NOTIFY hiddenConfigurationWindowsAreDeletedChanged)
    Q_PROPERTY(bool showInfoWindow READ showInfoWindow WRITE setShowInfoWindow NOTIFY showInfoWindowChanged)

    Q_PROPERTY(QString currentLayoutName READ currentLayoutName WRITE setCurrentLayoutName NOTIFY currentLayoutNameChanged)

    Q_PROPERTY(QStringList launchers READ launchers WRITE setLaunchers NOTIFY launchersChanged)

    Q_PROPERTY(Latte::Settings::MouseSensitivity sensitivity READ sensitivity WRITE setSensitivity NOTIFY sensitivityChanged)

    Q_PROPERTY(QQmlListProperty<QScreen> screens READ screens)

public:
    UniversalSettings(KSharedConfig::Ptr config, QObject *parent = nullptr);
    ~UniversalSettings() override;

    void load();

    bool autostart() const;
    void setAutostart(bool state);

    bool badges3DStyle() const;
    void setBadges3DStyle(bool enable);

    bool canDisableBorders() const;
    void setCanDisableBorders(bool enable);

    bool colorsScriptIsPresent() const;

    bool kwin_metaForwardedToLatte() const;
    void kwin_forwardMetaToLatte(bool forward);

    bool kwin_borderlessMaximizedWindowsEnabled() const;
    void kwin_setDisabledMaximizedBorders(bool disable);

    bool hiddenConfigurationWindowsAreDeleted() const;
    void setHiddenConfigurationWindowsAreDeleted(bool enabled);

    bool metaPressAndHoldEnabled() const;
    void setMetaPressAndHoldEnabled(bool enabled);

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

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    Settings::MouseSensitivity sensitivity();
    void setSensitivity(Settings::MouseSensitivity sense);

    QQmlListProperty<QScreen> screens();
    static int countScreens(QQmlListProperty<QScreen> *property); //! is needed by screens()
    static QScreen *atScreens(QQmlListProperty<QScreen> *property, int index); //! is needed by screens()

public slots:
    Q_INVOKABLE QString splitterIconPath();
    Q_INVOKABLE QString trademarkIconPath();

    Q_INVOKABLE float screenWidthScale(QString screenName) const;
    Q_INVOKABLE float screenHeightScale(QString screenName) const;
    Q_INVOKABLE void setScreenScales(QString screenName, float widthScale, float heightScale);

signals:
    void autostartChanged();
    void badges3DStyleChanged();
    void canDisableBordersChanged();
    void colorsScriptIsPresentChanged();
    void currentLayoutNameChanged();
    void downloadWindowSizeChanged();
    void hiddenConfigurationWindowsAreDeletedChanged();
    void lastNonAssignedLayoutNameChanged();
    void layoutsColumnWidthsChanged();
    void layoutsWindowSizeChanged();
    void launchersChanged();
    void layoutsMemoryUsageChanged();
    void metaPressAndHoldEnabledChanged();
    void sensitivityChanged();
    void screensCountChanged();
    void screenScalesChanged();
    void screenTrackerIntervalChanged();
    void showInfoWindowChanged();
    void versionChanged();

private slots:
    void loadConfig();
    void loadScalesConfig();
    void saveConfig();
    void saveScalesConfig();

    void recoverKWinOptions();
    void updateColorsScriptIsPresent();
    void trackedFileChanged(const QString &file);

private:
    void cleanupSettings();

    void setColorsScriptIsPresent(bool present);

    MemoryUsage::LayoutsMemory layoutsMemoryUsage() const;
    void setLayoutsMemoryUsage(MemoryUsage::LayoutsMemory layoutsMemoryUsage);

private:
    bool m_badges3DStyle{false};
    bool m_canDisableBorders{false};
    bool m_colorsScriptIsPresent{false};
    bool m_hiddenConfigurationWindowsAreDeleted{true};
    bool m_metaPressAndHoldEnabled{true};
    bool m_showInfoWindow{true};

    //!kwinrc tracking
    bool m_kwinMetaForwardedToLatte{false};
    bool m_kwinBorderlessMaximizedWindows{false};

    //when there isnt a version it is an old universal file
    int m_version{1};

    int m_screenTrackerInterval{2500};

    QString m_currentLayoutName;
    QString m_lastNonAssignedLayoutName;

    QStringList m_launchers;

    MemoryUsage::LayoutsMemory m_memoryUsage;
    Settings::MouseSensitivity m_sensitivity{Settings::HighMouseSensitivity};

    //! ScreenName, <width_scale, height_scale>
    QHash<QString, ScreenScales> m_screenScales;

    QPointer<Latte::Corona> m_corona;

    KConfigGroup m_screenScalesGroup;
    KConfigGroup m_universalGroup;
    KSharedConfig::Ptr m_config;

    //! reading kwinrc values is costly; a tracker protects from
    //! reading too many times with no real reason
    QTimer m_kwinrcTrackerTimer;

    friend class Layouts::Manager;
    friend class Latte::Corona;
};

}

#endif //UNIVERSALSETTINGS_H
