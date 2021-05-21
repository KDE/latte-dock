/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: Michail Vourlakos <mvourlakos@gmail.com>

    This file is part of Latte-Dock

    SPDX-License-Identifier: GPL-2.0-or-later
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
    Q_PROPERTY(bool inAdvancedModeForEditSettings READ inAdvancedModeForEditSettings WRITE setInAdvancedModeForEditSettings NOTIFY inAdvancedModeForEditSettingsChanged)
    Q_PROPERTY(bool colorsScriptIsPresent READ colorsScriptIsPresent NOTIFY colorsScriptIsPresentChanged)
    Q_PROPERTY(bool showInfoWindow READ showInfoWindow WRITE setShowInfoWindow NOTIFY showInfoWindowChanged)

    Q_PROPERTY(QString singleModeLayoutName READ singleModeLayoutName WRITE setSingleModeLayoutName NOTIFY singleModeLayoutNameChanged)

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

    bool inAdvancedModeForEditSettings() const;
    void setInAdvancedModeForEditSettings(const bool &inAdvanced);

    bool kwin_metaForwardedToLatte() const;
    void kwin_forwardMetaToLatte(bool forward);

    bool kwin_borderlessMaximizedWindowsEnabled() const;
    void kwin_setDisabledMaximizedBorders(bool disable);

    bool metaPressAndHoldEnabled() const;
    void setMetaPressAndHoldEnabled(bool enabled);

    bool showInfoWindow() const;
    void setShowInfoWindow(bool show);

    int version() const;
    void setVersion(int ver);

    int screenTrackerInterval() const;
    void setScreenTrackerInterval(int duration);

    QString singleModeLayoutName() const;
    void setSingleModeLayoutName(QString layoutName);

    QStringList launchers() const;
    void setLaunchers(QStringList launcherList);

    Settings::MouseSensitivity sensitivity();
    void setSensitivity(Settings::MouseSensitivity sense);

    QQmlListProperty<QScreen> screens();
    static int countScreens(QQmlListProperty<QScreen> *property); //! is needed by screens()
    static QScreen *atScreens(QQmlListProperty<QScreen> *property, int index); //! is needed by screens()

public slots:
    Q_INVOKABLE QString splitterIconPath();
    Q_INVOKABLE QString trademarkPath();
    Q_INVOKABLE QString trademarkIconPath();

    Q_INVOKABLE float screenWidthScale(QString screenName) const;
    Q_INVOKABLE float screenHeightScale(QString screenName) const;
    Q_INVOKABLE void setScreenScales(QString screenName, float widthScale, float heightScale);

    void syncSettings();

signals:
    void autostartChanged();
    void badges3DStyleChanged();
    void canDisableBordersChanged();
    void colorsScriptIsPresentChanged();
    void downloadWindowSizeChanged();
    void inAdvancedModeForEditSettingsChanged();
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
    void singleModeLayoutNameChanged();
    void versionChanged();

private slots:
    void loadConfig();
    void loadScalesConfig();
    void saveConfig();
    void saveScalesConfig();

    void recoverKWinOptions();
    void updateColorsScriptIsPresent();
    void trackedFileChanged(const QString &file);

    void upgrade_v010();
private:
    void cleanupSettings();

    void setColorsScriptIsPresent(bool present);

    MemoryUsage::LayoutsMemory layoutsMemoryUsage() const;
    void setLayoutsMemoryUsage(MemoryUsage::LayoutsMemory layoutsMemoryUsage);

private:
    bool m_badges3DStyle{false};
    bool m_canDisableBorders{false};
    bool m_colorsScriptIsPresent{false};
    bool m_inAdvancedModeForEditSettings{false};
    bool m_metaPressAndHoldEnabled{true};
    bool m_showInfoWindow{true};

    //!kwinrc tracking
    bool m_kwinMetaForwardedToLatte{false};
    bool m_kwinBorderlessMaximizedWindows{false};

    //when there isnt a version it is an old universal file
    int m_version{1};

    int m_screenTrackerInterval{2500};

    QString m_singleModeLayoutName;

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
    KSharedConfigPtr m_kwinrcPtr;
    KConfigGroup m_kwinrcModifierOnlyShortcutsGroup;
    KConfigGroup m_kwinrcWindowsGroup;

    friend class Layouts::Manager;
    friend class Latte::Corona;
};

}

#endif //UNIVERSALSETTINGS_H
