/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INDICATORFACTORY_H
#define INDICATORFACTORY_H

// local
#include "../apptypes.h"

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
    bool isCustomType(const QString &id) const;

    QString uiPath(QString pluginName) const;

    static QString metadataFileAbsolutePath(const QString &directoryPath);

    //! metadata record
    static bool metadataAreValid(KPluginMetaData &metadata);
    //! metadata file
    static bool metadataAreValid(QString &file);

    //! imports an indicator compressed file
    static Latte::ImportExport::State importIndicatorFile(QString compressedFile);
signals:
    void indicatorChanged(const QString &indicatorId);
    void indicatorRemoved(const QString &indicatorId);

private:
    void reload(const QString &indicatorPath);

    void removeIndicatorRecords(const QString &path);
    void discoverNewIndicators(const QString &main);

private:
    QHash<QString, KPluginMetaData> m_plugins;
    QHash<QString, QString> m_pluginUiPaths;

    QStringList m_customPluginIds;
    QStringList m_customPluginNames;
    QStringList m_customLocalPluginIds;

    //! plugins paths
    QStringList m_mainPaths;
    QStringList m_indicatorsPaths;

    QWidget *m_parentWidget;
};

}
}

#endif
