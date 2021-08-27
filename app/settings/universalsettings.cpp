/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "universalsettings.h"

// local
#include "../data/contextmenudata.h"
#include "../data/layoutdata.h"
#include "../layout/centrallayout.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../tools/commontools.h"

// Qt
#include <QDebug>
#include <QDir>
#include <QtDBus>

// KDE
#include <KActivities/Consumer>
#include <KDirWatch>
#include <KWindowSystem>

#define KWINMETAFORWARDTOLATTESTRING "org.kde.lattedock,/Latte,org.kde.LatteDock,activateLauncherMenu"
#define KWINMETAFORWARDTOPLASMASTRING "org.kde.plasmashell,/PlasmaShell,org.kde.PlasmaShell,activateLauncherMenu"

#define KWINCOLORSSCRIPT "kwin/scripts/lattewindowcolors"
#define KWINRC "kwinrc"

#define KWINRCTRACKERINTERVAL 2500

namespace Latte {

UniversalSettings::UniversalSettings(KSharedConfig::Ptr config, QObject *parent)
    : QObject(parent),
      m_config(config),
      m_universalGroup(KConfigGroup(config, QStringLiteral("UniversalSettings")))
{
    m_corona = qobject_cast<Latte::Corona *>(parent);

    connect(this, &UniversalSettings::actionsChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::badges3DStyleChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::canDisableBordersChanged, this, &UniversalSettings::saveConfig);  
    connect(this, &UniversalSettings::inAdvancedModeForEditSettingsChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::isAvailableGeometryBroadcastedToPlasmaChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::launchersChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::layoutsMemoryUsageChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::metaPressAndHoldEnabledChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::sensitivityChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::screenTrackerIntervalChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::showInfoWindowChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::singleModeLayoutNameChanged, this, &UniversalSettings::saveConfig);
    connect(this, &UniversalSettings::versionChanged, this, &UniversalSettings::saveConfig);

    connect(this, &UniversalSettings::screenScalesChanged, this, &UniversalSettings::saveScalesConfig);

    connect(qGuiApp, &QGuiApplication::screenAdded, this, &UniversalSettings::screensCountChanged);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &UniversalSettings::screensCountChanged);

    m_kwinrcPtr = KSharedConfig::openConfig(Latte::configPath() + "/" + KWINRC);
    m_kwinrcModifierOnlyShortcutsGroup = KConfigGroup(m_kwinrcPtr, QStringLiteral("ModifierOnlyShortcuts"));
    m_kwinrcWindowsGroup = KConfigGroup(m_kwinrcPtr, QStringLiteral("Windows"));
}

UniversalSettings::~UniversalSettings()
{
    saveConfig();
    cleanupSettings();
}

void UniversalSettings::load()
{
    //! check if user has set the autostart option
    bool autostartUserSet = m_universalGroup.readEntry("userConfiguredAutostart", false);

    if (!autostartUserSet && !autostart()) {
        setAutostart(true);
    }

    //! init screen scales
    m_screenScalesGroup = m_universalGroup.group("ScreenScales");

    //! load configuration
    loadConfig();

    //! Track KWin Colors Script Presence
    updateColorsScriptIsPresent();

    QStringList colorsScriptPaths = Layouts::Importer::standardPathsFor(KWINCOLORSSCRIPT);
    for(auto path: colorsScriptPaths) {
        KDirWatch::self()->addDir(path);
    }

    //! Track KWin rc options
    const QString kwinrcFilePath = Latte::configPath() + "/" + KWINRC;
    KDirWatch::self()->addFile(kwinrcFilePath);
    recoverKWinOptions();

    m_kwinrcTrackerTimer.setSingleShot(true);
    m_kwinrcTrackerTimer.setInterval(KWINRCTRACKERINTERVAL);
    connect(&m_kwinrcTrackerTimer, &QTimer::timeout, this, &UniversalSettings::recoverKWinOptions);

    connect(KDirWatch::self(), &KDirWatch::created, this, &UniversalSettings::trackedFileChanged);
    connect(KDirWatch::self(), &KDirWatch::deleted, this, &UniversalSettings::trackedFileChanged);
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &UniversalSettings::trackedFileChanged);

    //! this is needed to inform globalshortcuts to update its modifiers tracking
    emit metaPressAndHoldEnabledChanged();
}

bool UniversalSettings::inAdvancedModeForEditSettings() const
{
    return m_inAdvancedModeForEditSettings;
}

void UniversalSettings::setInAdvancedModeForEditSettings(const bool &inAdvanced)
{
    if (m_inAdvancedModeForEditSettings == inAdvanced) {
        return;
    }

    m_inAdvancedModeForEditSettings = inAdvanced;
    emit inAdvancedModeForEditSettingsChanged();
}

bool UniversalSettings::isAvailableGeometryBroadcastedToPlasma() const
{
    return m_isAvailableGeometryBroadcastedToPlasma;
}

void UniversalSettings::setIsAvailableGeometryBroadcastedToPlasma(const bool &isBroadcasted)
{
    if (m_isAvailableGeometryBroadcastedToPlasma == isBroadcasted) {
        return;
    }

    m_isAvailableGeometryBroadcastedToPlasma = isBroadcasted;
    emit isAvailableGeometryBroadcastedToPlasmaChanged();
}

bool UniversalSettings::showInfoWindow() const
{
    return m_showInfoWindow;
}

void UniversalSettings::setShowInfoWindow(bool show)
{
    if (m_showInfoWindow == show) {
        return;
    }

    m_showInfoWindow = show;
    emit showInfoWindowChanged();
}

int UniversalSettings::version() const
{
    return m_version;
}

void UniversalSettings::setVersion(int ver)
{
    if (m_version == ver) {
        return;
    }

    m_version = ver;
    qDebug() << "Universal Settings version updated to : " << m_version;

    emit versionChanged();
}

int UniversalSettings::screenTrackerInterval() const
{
    return m_screenTrackerInterval;
}

void UniversalSettings::setScreenTrackerInterval(int duration)
{
    if (m_screenTrackerInterval == duration) {
        return;
    }

    m_screenTrackerInterval = duration;
    emit screenTrackerIntervalChanged();
}

QString UniversalSettings::singleModeLayoutName() const
{
    return m_singleModeLayoutName;
}

void UniversalSettings::setSingleModeLayoutName(QString layoutName)
{
    if (m_singleModeLayoutName == layoutName) {
        return;
    }

    m_singleModeLayoutName = layoutName;
    emit singleModeLayoutNameChanged();
}

QStringList UniversalSettings::contextMenuActionsAlwaysShown() const
{
    return m_contextMenuActionsAlwaysShown;
}

void UniversalSettings::setContextMenuActionsAlwaysShown(const QStringList &actions)
{
    if (m_contextMenuActionsAlwaysShown == actions) {
        return;
    }

    m_contextMenuActionsAlwaysShown = actions;
    emit actionsChanged();
}

QStringList UniversalSettings::launchers() const
{
    return m_launchers;
}

void UniversalSettings::setLaunchers(QStringList launcherList)
{
    if (m_launchers == launcherList) {
        return;
    }

    m_launchers = launcherList;
    emit launchersChanged();
}


bool UniversalSettings::autostart() const
{
    QFile autostartFile(Latte::configPath() + "/autostart/org.kde.latte-dock.desktop");
    return autostartFile.exists();
}

void UniversalSettings::setAutostart(bool state)
{
    //! remove old autostart file
    QFile oldAutostartFile(Latte::configPath() + "/autostart/latte-dock.desktop");

    if (oldAutostartFile.exists()) {
        oldAutostartFile.remove();
    }

    //! end of removal of old autostart file

    QFile autostartFile(Latte::configPath() + "/autostart/org.kde.latte-dock.desktop");
    QFile metaFile(Layouts::Importer::standardPath("applications/org.kde.latte-dock.desktop", false));

    if (!state && autostartFile.exists()) {
        //! the first time that the user disables the autostart, this is recorded
        //! and from now own it will not be recreated it in the beginning
        if (!m_universalGroup.readEntry("userConfiguredAutostart", false)) {
            m_universalGroup.writeEntry("userConfiguredAutostart", true);
        }

        autostartFile.remove();
        emit autostartChanged();
    } else if (state && metaFile.exists()) {
        //! check if autostart folder exists and create otherwise
        QDir autostartDir(Latte::configPath() + "/autostart");
        if (!autostartDir.exists()) {
            QDir configDir(Latte::configPath());
            configDir.mkdir("autostart");
        }

        metaFile.copy(autostartFile.fileName());
        //! I haven't added the flag "OnlyShowIn=KDE;" into the autostart file
        //! because I fall onto a Plasma 5.8 case that this flag
        //! didn't let the plasma desktop to start
        emit autostartChanged();
    }
}

bool UniversalSettings::badges3DStyle() const
{
    return m_badges3DStyle;
}

void UniversalSettings::setBadges3DStyle(bool enable)
{
    if (m_badges3DStyle == enable) {
        return;
    }

    m_badges3DStyle = enable;
    emit badges3DStyleChanged();
}


bool UniversalSettings::canDisableBorders() const
{
    return m_canDisableBorders;
}

void UniversalSettings::setCanDisableBorders(bool enable)
{
    if (m_canDisableBorders == enable) {
        return;
    }

    m_canDisableBorders = enable;
    emit canDisableBordersChanged();
}

bool UniversalSettings::colorsScriptIsPresent() const
{
    return m_colorsScriptIsPresent;
}

void UniversalSettings::setColorsScriptIsPresent(bool present)
{
    if (m_colorsScriptIsPresent == present) {
        return;
    }

    m_colorsScriptIsPresent = present;
    emit colorsScriptIsPresentChanged();
}

void UniversalSettings::updateColorsScriptIsPresent()
{
    qDebug() << "Updating Latte Colors Script presence...";

    setColorsScriptIsPresent(!Layouts::Importer::standardPath(KWINCOLORSSCRIPT).isEmpty());
}

void UniversalSettings::trackedFileChanged(const QString &file)
{    
    if (file.endsWith(KWINCOLORSSCRIPT)) {
        updateColorsScriptIsPresent();
    }

    if (file.endsWith(KWINRC)) {
        m_kwinrcTrackerTimer.start();
    }
}

bool UniversalSettings::kwin_metaForwardedToLatte() const
{
    return m_kwinMetaForwardedToLatte;
}

bool UniversalSettings::kwin_borderlessMaximizedWindowsEnabled() const
{
    return m_kwinBorderlessMaximizedWindows;
}

void UniversalSettings::kwin_forwardMetaToLatte(bool forward)
{
    if (m_kwinMetaForwardedToLatte == forward) {
        return;
    }

    if (KWindowSystem::isPlatformWayland()) {
        // BUG: https://bugs.kde.org/show_bug.cgi?id=428202
        // KWin::reconfigure() function blocks/freezes Latte under wayland
        return;
    }

    QString forwardStr = (forward ? KWINMETAFORWARDTOLATTESTRING : KWINMETAFORWARDTOPLASMASTRING);
    m_kwinrcModifierOnlyShortcutsGroup.writeEntry("Meta", forwardStr);
    m_kwinrcModifierOnlyShortcutsGroup.sync();

    QDBusInterface iface("org.kde.KWin", "/KWin", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("reconfigure");
    }
}

void UniversalSettings::kwin_setDisabledMaximizedBorders(bool disable)
{
    if (m_kwinBorderlessMaximizedWindows == disable) {
        return;
    }

    if (KWindowSystem::isPlatformWayland()) {
        // BUG: https://bugs.kde.org/show_bug.cgi?id=428202
        // KWin::reconfigure() function blocks/freezes Latte under wayland
        return;
    }

    m_kwinrcWindowsGroup.writeEntry("BorderlessMaximizedWindows", disable);
    m_kwinrcWindowsGroup.sync();

    QDBusInterface iface("org.kde.KWin", "/KWin", "", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.call("reconfigure");
        m_kwinBorderlessMaximizedWindows = disable;
    }
}

void UniversalSettings::recoverKWinOptions()
{
    qDebug() << "kwinrc: recovering values...";

    //! Meta forwarded to Latte option
    QString metaforwardedstr = m_kwinrcModifierOnlyShortcutsGroup.readEntry("Meta", KWINMETAFORWARDTOPLASMASTRING);
    m_kwinMetaForwardedToLatte = (metaforwardedstr == KWINMETAFORWARDTOLATTESTRING);

    //! BorderlessMaximizedWindows option
    m_kwinBorderlessMaximizedWindows = m_kwinrcWindowsGroup.readEntry("BorderlessMaximizedWindows", false);
}

bool UniversalSettings::metaPressAndHoldEnabled() const
{
    return m_metaPressAndHoldEnabled;
}

void UniversalSettings::setMetaPressAndHoldEnabled(bool enabled)
{
    if (m_metaPressAndHoldEnabled == enabled) {
        return;
    }

    m_metaPressAndHoldEnabled = enabled;

    emit metaPressAndHoldEnabledChanged();
}

MemoryUsage::LayoutsMemory UniversalSettings::layoutsMemoryUsage() const
{
    return m_memoryUsage;
}

void UniversalSettings::setLayoutsMemoryUsage(MemoryUsage::LayoutsMemory layoutsMemoryUsage)
{
    if (m_memoryUsage == layoutsMemoryUsage) {
        return;
    }

    m_memoryUsage = layoutsMemoryUsage;
    emit layoutsMemoryUsageChanged();
}

Settings::MouseSensitivity UniversalSettings::sensitivity()
{
    return m_sensitivity;
}

void UniversalSettings::setSensitivity(Settings::MouseSensitivity sense)
{
    if (m_sensitivity == sense) {
        return;
    }

    m_sensitivity = sense;
    emit sensitivityChanged();
}

float UniversalSettings::screenWidthScale(QString screenName) const
{
    if (!m_screenScales.contains(screenName)) {
        return 1;
    }

    return m_screenScales[screenName].first;
}

float UniversalSettings::screenHeightScale(QString screenName) const
{
    if (!m_screenScales.contains(screenName)) {
        return 1;
    }

    return m_screenScales[screenName].second;
}

void UniversalSettings::setScreenScales(QString screenName, float widthScale, float heightScale)
{
    if (!m_screenScales.contains(screenName)) {
        m_screenScales[screenName].first = widthScale;
        m_screenScales[screenName].second = heightScale;
    } else {
        if (m_screenScales[screenName].first == widthScale
                && m_screenScales[screenName].second == heightScale) {
            return;
        }

        m_screenScales[screenName].first = widthScale;
        m_screenScales[screenName].second = heightScale;
    }

    emit screenScalesChanged();
}

void UniversalSettings::syncSettings()
{
    m_universalGroup.sync();
}

void UniversalSettings::upgrade_v010()
{
    if (m_singleModeLayoutName.isEmpty()) {
        //!Upgrading path for v0.9 to v0.10
        QString lastNonAssigned = m_universalGroup.readEntry("lastNonAssignedLayout", QString());
        QString currentLayout = m_universalGroup.readEntry("currentLayout", QString());

        if (!lastNonAssigned.isEmpty()) {
            m_singleModeLayoutName = lastNonAssigned;
        } else if (!currentLayout.isEmpty()) {
            m_singleModeLayoutName = currentLayout;
        }

        if (!m_singleModeLayoutName.isEmpty() && Layouts::Importer::layoutExists(m_singleModeLayoutName)) {
            //! it is executed only after the upgrade path
            m_universalGroup.writeEntry("singleModeLayoutName", m_singleModeLayoutName);
            CentralLayout storage(this, Layouts::Importer::layoutUserFilePath(m_singleModeLayoutName));
            if (m_singleModeLayoutName == lastNonAssigned) {
                storage.setActivities(QStringList(Data::Layout::FREEACTIVITIESID));
            } else if (storage.activities().isEmpty()) {
                storage.setActivities(QStringList(Data::Layout::ALLACTIVITIESID));
            }
        }
    }
}

void UniversalSettings::loadConfig()
{
    m_version = m_universalGroup.readEntry("version", 1);
    m_badges3DStyle = m_universalGroup.readEntry("badges3DStyle", false);
    m_canDisableBorders = m_universalGroup.readEntry("canDisableBorders", false);
    m_contextMenuActionsAlwaysShown = m_universalGroup.readEntry("contextMenuActionsAlwaysShown", Latte::Data::ContextMenu::ACTIONSALWAYSVISIBLE);
    m_inAdvancedModeForEditSettings = m_universalGroup.readEntry("inAdvancedModeForEditSettings", false);
    m_isAvailableGeometryBroadcastedToPlasma = m_universalGroup.readEntry("isAvailableGeometryBroadcastedToPlasma", true);
    m_launchers = m_universalGroup.readEntry("launchers", QStringList());
    m_metaPressAndHoldEnabled = m_universalGroup.readEntry("metaPressAndHoldEnabled", true);
    m_screenTrackerInterval = m_universalGroup.readEntry("screenTrackerInterval", 2500);
    m_showInfoWindow = m_universalGroup.readEntry("showInfoWindow", true);
    m_singleModeLayoutName = m_universalGroup.readEntry("singleModeLayoutName", QString());
    m_memoryUsage = static_cast<MemoryUsage::LayoutsMemory>(m_universalGroup.readEntry("memoryUsage", (int)MemoryUsage::SingleLayout));
    m_sensitivity = static_cast<Settings::MouseSensitivity>(m_universalGroup.readEntry("mouseSensitivity", (int)Settings::HighMouseSensitivity));

    loadScalesConfig();

    if (m_singleModeLayoutName.isEmpty()) {
        upgrade_v010();
    }
}

void UniversalSettings::saveConfig()
{
    m_universalGroup.writeEntry("version", m_version);
    m_universalGroup.writeEntry("badges3DStyle", m_badges3DStyle);
    m_universalGroup.writeEntry("canDisableBorders", m_canDisableBorders);
    m_universalGroup.writeEntry("contextMenuActionsAlwaysShown", m_contextMenuActionsAlwaysShown);
    m_universalGroup.writeEntry("inAdvancedModeForEditSettings", m_inAdvancedModeForEditSettings);
    m_universalGroup.writeEntry("isAvailableGeometryBroadcastedToPlasma", m_isAvailableGeometryBroadcastedToPlasma);
    m_universalGroup.writeEntry("launchers", m_launchers);
    m_universalGroup.writeEntry("metaPressAndHoldEnabled", m_metaPressAndHoldEnabled);
    m_universalGroup.writeEntry("screenTrackerInterval", m_screenTrackerInterval);
    m_universalGroup.writeEntry("showInfoWindow", m_showInfoWindow);
    m_universalGroup.writeEntry("singleModeLayoutName", m_singleModeLayoutName);
    m_universalGroup.writeEntry("memoryUsage", (int)m_memoryUsage);
    m_universalGroup.writeEntry("mouseSensitivity", (int)m_sensitivity);
    syncSettings();
}

void UniversalSettings::cleanupSettings()
{
    KConfigGroup containments = KConfigGroup(m_config, QStringLiteral("Containments"));
    containments.deleteGroup();

    containments.sync();
}

QString UniversalSettings::splitterIconPath()
{
    return m_corona->kPackage().filePath("splitter");
}

QString UniversalSettings::trademarkPath()
{
    return m_corona->kPackage().filePath("trademark");
}

QString UniversalSettings::trademarkIconPath()
{
    return m_corona->kPackage().filePath("trademarkicon");
}

QQmlListProperty<QScreen> UniversalSettings::screens()
{
    return QQmlListProperty<QScreen>(this, nullptr, &countScreens, &atScreens);
}

int UniversalSettings::countScreens(QQmlListProperty<QScreen> *property)
{
    Q_UNUSED(property)
    return qGuiApp->screens().count();
}

QScreen *UniversalSettings::atScreens(QQmlListProperty<QScreen> *property, int index)
{
    Q_UNUSED(property)
    return qGuiApp->screens().at(index);
}

void UniversalSettings::loadScalesConfig()
{
    for (const auto &screenName : m_screenScalesGroup.keyList()) {
        QString scalesStr = m_screenScalesGroup.readEntry(screenName, QString());
        QStringList scales = scalesStr.split(";");
        if (scales.count() == 2) {
            m_screenScales[screenName] = qMakePair(scales[0].toFloat(), scales[1].toFloat());
        }
    }
}

void UniversalSettings::saveScalesConfig()
{
    for (const auto &screenName : m_screenScales.keys()) {
        QStringList scales;
        scales << QString::number(m_screenScales[screenName].first) << QString::number(m_screenScales[screenName].second);
        m_screenScalesGroup.writeEntry(screenName, scales.join(";"));
    }

    m_screenScalesGroup.sync();
}

}
