/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
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

#include "abstractwindowinterface.h"
#include "xwindowinterface.h"
#include "waylandinterface.h"

#include <QObject>
#include <QDir>
#include <QQuickWindow>

#include <KDirWatch>
#include <KWindowSystem>

namespace Latte {

AbstractWindowInterface::AbstractWindowInterface(QObject *parent)
    : QObject(parent)
{
    updateDefaultScheme();

    connect(this, &AbstractWindowInterface::windowRemoved, this, [&](WindowId wid) {
        m_windowScheme.remove(wid);
    });

    //! track for changing default scheme
    QString kdeSettingsFile = QDir::homePath() + "/.config/kdeglobals";

    KDirWatch::self()->addFile(kdeSettingsFile);

    connect(KDirWatch::self(), &KDirWatch::dirty, this, [ &, kdeSettingsFile](const QString & path) {
        if (path == kdeSettingsFile) {
            this->updateDefaultScheme();
        }
    });

    connect(KDirWatch::self(), &KDirWatch::created, this, [ &, kdeSettingsFile](const QString & path) {
        if (path == kdeSettingsFile) {
            this->updateDefaultScheme();
        }
    });
}

AbstractWindowInterface::~AbstractWindowInterface()
{
    m_windowScheme.clear();
    //! it is just a reference to a real scheme file
    m_schemes.take("kdeglobals");
    qDeleteAll(m_schemes);
    m_schemes.clear();
}

void AbstractWindowInterface::addDock(WindowId wid)
{
    m_docks.push_back(wid);
}

void AbstractWindowInterface::removeDock(WindowId wid)
{
    auto it = std::find(m_docks.begin(), m_docks.end(), wid);

    if (it != m_docks.end())
        m_docks.erase(it);
}


//! Scheme support for windows
void AbstractWindowInterface::updateDefaultScheme()
{
    QString defaultSchemePath = SchemeColors::possibleSchemeFile("kdeglobals");

    SchemeColors *dScheme;

    if (!m_schemes.contains(defaultSchemePath)) {
        dScheme = new SchemeColors(this, defaultSchemePath);
    } else {
        dScheme = m_schemes[defaultSchemePath];
    }

    if (!m_schemes.contains("kdeglobal") || m_schemes["kdeglobals"]->schemeFile() != defaultSchemePath) {
        m_schemes["kdeglobals"] = dScheme;
    }
}

SchemeColors *AbstractWindowInterface::schemeForWindow(WindowId wid)
{
    if (!m_windowScheme.contains(wid)) {
        return m_schemes["kdeglobals"];
    } else {
        return m_schemes[m_windowScheme[wid]];
    }

    return nullptr;
}

void AbstractWindowInterface::setColorSchemeForWindow(WindowId wid, QString scheme)
{
    if (scheme == "kdeglobals" && !m_windowScheme.contains(wid)) {
        //default scheme does not have to be set
        return;
    }

    if (scheme == "kdeglobals") {
        //! a window that previously had an explicit set scheme now is set back to default scheme
        m_windowScheme.remove(wid);
    } else {
        QString schemeFile = SchemeColors::possibleSchemeFile(scheme);

        if (!m_schemes.contains(schemeFile)) {
            //! when this scheme file has not been loaded yet
            m_schemes[schemeFile] = new SchemeColors(this, schemeFile);
        }

        m_windowScheme[wid] = schemeFile;

        if (wid == activeWindow()) {
            emit activeWindowChanged(wid);
        }
    }
}

}

