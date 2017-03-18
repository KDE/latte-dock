#include "globalsettings.h"

#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QFile>

#include <KLocalizedString>

namespace Latte {

GlobalSettings::GlobalSettings(QObject *parent)
    : QObject(parent)
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        m_configGroup = m_corona->config()->group("General");

        //! create the alternative session action
        const QIcon altIcon = QIcon::fromTheme("user-identity");
        m_altSessionAction = new QAction(altIcon, i18n("Alternative Session"), this);
        m_altSessionAction->setStatusTip(tr("Enable/Disable Alternative Session"));
        m_altSessionAction->setCheckable(true);
        connect(m_altSessionAction, &QAction::triggered, this, &GlobalSettings::enableAltSession);

        connect(m_corona, &DockCorona::currentSessionChanged, this, &GlobalSettings::currentSessionChangedSlot);
    }
}

GlobalSettings::~GlobalSettings()
{
    m_altSessionAction->deleteLater();
    m_configGroup.sync();
}

void GlobalSettings::enableAltSession(bool enabled)
{
    if (enabled) {
        m_corona->switchToSession(Dock::AlternativeSession);
    } else {
        m_corona->switchToSession(Dock::DefaultSession);
    }
}

bool GlobalSettings::exposeAltSession() const
{
    return m_exposeAltSession;
}

void GlobalSettings::setExposeAltSession(bool state)
{
    if (m_exposeAltSession == state) {
        return;
    }

    m_exposeAltSession = state;
    save();
    emit exposeAltSessionChanged();
}

void GlobalSettings::currentSessionChangedSlot(Dock::SessionType type)
{
    if (m_corona->currentSession() == Dock::DefaultSession)
        m_altSessionAction->setChecked(false);
    else
        m_altSessionAction->setChecked(true);

    emit currentSessionChanged();
}

QAction *GlobalSettings::altSessionAction() const
{
    return m_altSessionAction;
}

bool GlobalSettings::autostart() const
{
    QFile autostartFile(QDir::homePath() + "/.config/autostart/latte-dock.desktop");
    return autostartFile.exists();
}

void GlobalSettings::setAutostart(bool state)
{
    QFile autostartFile(QDir::homePath() + "/.config/autostart/latte-dock.desktop");
    QFile metaFile("/usr/share/applications/latte-dock.desktop");

    if (!state && autostartFile.exists()) {
        autostartFile.remove();
        emit autostartChanged();
    } else if (state && metaFile.exists()) {
        metaFile.copy(autostartFile.fileName());
        //! I havent added the flag "OnlyShowIn=KDE;" into the autostart file
        //! because I fall onto a Plasma 5.8 case that this flag
        //! didnt let the plasma desktop to start
        emit autostartChanged();
    }
}

Dock::SessionType GlobalSettings::currentSession() const
{
    return m_corona->currentSession();
}

void GlobalSettings::setCurrentSession(Dock::SessionType session)
{
    if (currentSession() != session) {
        m_corona->switchToSession(session);
    }
}


//!BEGIN configuration functions
void GlobalSettings::load()
{
    setExposeAltSession(m_configGroup.readEntry("exposeAltSession", false));
}

void GlobalSettings::save()
{
    m_configGroup.writeEntry("exposeAltSession", m_exposeAltSession);
    m_configGroup.sync();
}
//!END configuration functions

}

#include "moc_globalsettings.cpp"
