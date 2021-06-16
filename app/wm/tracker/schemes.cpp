/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "schemes.h"

// local
#include "../abstractwindowinterface.h"
#include "../../lattecorona.h"
#include "../../tools/commontools.h"

// Qt
#include <QDir>
#include <QLatin1String>

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
    qDeleteAll(m_schemes.values());
    m_schemes.clear();
}

void Schemes::init()
{
    updateDefaultScheme();

    connect(this, &Schemes::colorSchemeChanged, this, [&](WindowId wid) {
        if (wid == m_wm->activeWindow()) {
            emit m_wm->activeWindowChanged(wid);
        }
    });

    connect(m_wm, &AbstractWindowInterface::windowRemoved, this, [&](WindowId wid) {
        m_windowScheme.remove(wid);
    });

    //! track for changing default scheme
    QString kdeSettingsFile = Latte::configPath() + "/kdeglobals";

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

    emit defaultSchemeChanged();
}

SchemeColors *Schemes::schemeForFile(const QString &scheme)
{
    QString schemeFile = SchemeColors::possibleSchemeFile(scheme);

    if (!schemeFile.isEmpty() && !m_schemes.contains(schemeFile)) {
        //! when this scheme file has not been loaded yet
        m_schemes[schemeFile] = new SchemeColors(this, schemeFile);
    }

    return m_schemes.contains(schemeFile) ? m_schemes[schemeFile] : nullptr;
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
    if (scheme == QLatin1String("kdeglobals") && !m_windowScheme.contains(wid)) {
        //default scheme does not have to be set
        return;
    }

    if (scheme == QLatin1String("kdeglobals")) {
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

    emit colorSchemeChanged(wid);
}

}
}
}
