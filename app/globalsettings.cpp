#include "globalsettings.h"

#include <QIcon>
#include <QDebug>

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

        connect(m_corona, &DockCorona::currentSessionChanged, this, &GlobalSettings::currentSessionChanged);
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

void GlobalSettings::currentSessionChanged(Dock::SessionType type)
{
    if (m_corona->currentSession() == Dock::DefaultSession)
        m_altSessionAction->setChecked(false);
    else
        m_altSessionAction->setChecked(true);

}

QAction *GlobalSettings::altSessionAction() const
{
    return m_altSessionAction;
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
