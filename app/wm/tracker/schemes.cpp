/*
*  Copyright 2019  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "schemes.h"

// local
#include "../abstractwindowinterface.h"
#include "../../lattecorona.h"

// Qt
#include <QDir>

// KDE
#include <KDirWatch>


namespace Latte {
namespace WindowSystem {
namespace Tracker {

Schemes::Schemes(AbstractWindowInterface *parent)
    : QObject(parent)
{
    m_wm = parent;
    init();
}

Schemes::~Schemes()
{
    m_windowScheme.clear();
    //! it is just a reference to a real scheme file
    m_schemes.take("kdeglobals");
    qDeleteAll(m_schemes);
    m_schemes.clear();
}

void Schemes::init()
{
    updateDefaultScheme();

    connect(m_wm, &AbstractWindowInterface::windowRemoved, this, [&](WindowId wid) {
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

//! Scheme support for windows
void Schemes::updateDefaultScheme()
{
    QString defaultSchemePath = SchemeColors::possibleSchemeFile("kdeglobals");

    qDebug() << " Windows default color scheme :: " << defaultSchemePath;

    SchemeColors *dScheme;

    if (!m_schemes.contains(defaultSchemePath)) {
        dScheme = new SchemeColors(this, defaultSchemePath);
        m_schemes[defaultSchemePath] = dScheme;
    } else {
        dScheme = m_schemes[defaultSchemePath];
    }

    if (!m_schemes.contains("kdeglobals") || m_schemes["kdeglobals"]->schemeFile() != defaultSchemePath) {
        m_schemes["kdeglobals"] = dScheme;
    }
}

SchemeColors *Schemes::schemeForWindow(WindowId wid)
{
    if (!m_windowScheme.contains(wid)) {
        return m_schemes["kdeglobals"];
    } else {
        return m_schemes[m_windowScheme[wid]];
    }

    return nullptr;
}

void Schemes::setColorSchemeForWindow(WindowId wid, QString scheme)
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
    }

    if (wid == m_wm->activeWindow()) {
        emit m_wm->activeWindowChanged(wid);
    }
}

}
}
}
