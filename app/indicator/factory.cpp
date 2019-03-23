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

#include "factory.h"

// local
#include "../importer.h"

// Qt
#include <QDebug>
#include <QDir>

// KDE
#include <KPluginMetaData>

namespace Latte {
namespace Indicator {

Factory::Factory(QObject *parent)
    : QObject(parent)
{
    reload();

    qDebug() << m_plugins["org.kde.latte.indicator.default"].name();
}

Factory::~Factory()
{
}

KPluginMetaData Factory::metadata(QString pluginId)
{
    if (m_plugins.contains(pluginId)) {
        return m_plugins[pluginId];
    }

    return KPluginMetaData();
}

void Factory::reload()
{
    QStringList standardPaths = Latte::Importer::standardPaths();

    m_plugins.clear();

    foreach(auto path, standardPaths) {
        QDir standard(path + "/latte/indicators");

        if (standard.exists()) {
            QStringList pluginDirs = standard.entryList(QStringList(),QDir::AllDirs | QDir::NoSymLinks);

            foreach (auto pluginDir, pluginDirs) {
                if (pluginDir != "." && pluginDir != "..") {
                    QString metadataFile = standard.absolutePath() + "/" + pluginDir + "/metadata.desktop";

                    if (QFileInfo(metadataFile).exists()) {

                        KPluginMetaData metadata = KPluginMetaData::fromDesktopFile(metadataFile);
                        QString uiFile = standard.absolutePath() + "/" + pluginDir + "/package/" + metadata.value("X-Latte-MainScript");

                        if (metadata.isValid() && QFileInfo(uiFile).exists() && !m_plugins.contains(metadata.pluginId())) {
                            m_plugins[metadata.pluginId()] = metadata;
                            QString pluginPath = metadata.fileName().remove("metadata.desktop");
                            qDebug() << " Indicator Package Loaded ::: " << metadata.name() << " [" << metadata.pluginId() << "]" << " - [" <<pluginPath<<"]";
                            /*qDebug() << " Indicator value ::: " << metadata.pluginId();
                            qDebug() << " Indicator value ::: " << metadata.fileName();
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-MainScript");
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-ConfigUi");
                            qDebug() << " Indicator value ::: " << metadata.value("X-Latte-ConfigXml");*/
                        }
                    }
                }
            }
        }
    }
}

}
}
