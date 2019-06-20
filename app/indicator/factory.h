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

#ifndef INDICATORFACTORY_H
#define INDICATORFACTORY_H

// local
#include "../../liblatte2/types.h"

// Qt
#include <QHash>
#include <QObject>
#include <QWidget>

class KPluginMetaData;

namespace Latte {
namespace Indicator {

class Factory : public QObject
{
    Q_OBJECT

public:
    Factory(QObject *parent);
    ~Factory() override;

    int customPluginsCount();
    QStringList customPluginIds();
    QStringList customPluginNames();
    QStringList customLocalPluginIds();

    KPluginMetaData metadata(QString pluginId);

    void downloadIndicator();
    void removeIndicator(QString id);

    bool pluginExists(QString id) const;

    QString uiPath(QString pluginName) const;

    //! metadata record
    static bool metadataAreValid(KPluginMetaData &metadata);
    //! metadata file
    static bool metadataAreValid(QString &file);

    //! imports an indicator compressed file
    static Latte::Types::ImportExportState importIndicatorFile(QString compressedFile);
signals:
    void customPluginsChanged();
    void pluginsUpdated();

private:
    void reload();

private:
    QHash<QString, KPluginMetaData> m_plugins;
    QHash<QString, QString> m_pluginUiPaths;

    QStringList m_customPluginIds;
    QStringList m_customPluginNames;
    QStringList m_customLocalPluginIds;

    QStringList m_watchedPaths;

    QWidget *m_parentWidget;
};

}
}

#endif
