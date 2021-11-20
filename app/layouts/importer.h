/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTER_H
#define IMPORTER_H

// local
#include "../apptypes.h"

// Qt
#include <QObject>
#include <QTemporaryDir>

namespace Latte {
namespace Layouts {
class Manager;
}
}

namespace Latte {
namespace Layouts {

//! This class is responsible to import/export configurations
//! and of course to import old configuration to new architecture
class Importer : public QObject
{
    Q_OBJECT

public:
    enum LatteFileVersion
    {
        UnknownFileType = -1,
        LayoutVersion1 = 0,
        ConfigVersion1 = 1,
        LayoutVersion2 = 2,
        ConfigVersion2 = 3
    };
    Q_ENUM(LatteFileVersion);

    Importer(QObject *parent = nullptr);
    ~Importer() override;

    //! updates the old configuration to version: 2
    bool updateOldConfiguration();

    //! imports an old layout file,
    //! newName: the layout new name, if it is empty the original is used
    //! alternative: old files can contain both a Default and an Alternative layout
    //!    false: imports only Default layout
    //!    true: imports only Alternative layout
    bool importOldLayout(QString oldAppletsPath, QString newName, bool alternative = false, QString exportDirectory = QString());

    //! imports and old configuration file (tar archive) that contains
    //! both an applets file and a latterc file with the screens
    //!     newName: if it is empty the name is extracted from the old config file name
    bool importOldConfiguration(QString oldConfigPath, QString newName = QString());

    bool exportFullConfiguration(QString file);

    QString storageTmpDir() const;
    //! imports the specific layout and return the new layout name.
    //! if the function didn't succeed returns an empty string
    QString importLayout(const QString &fileName, const QString &suggestedName = QString());

    static void enableAutostart();
    static void disableAutostart();
    static bool isAutostartEnabled();

    static Importer::LatteFileVersion fileVersion(QString file);

    static bool importHelper(QString fileName);

    //! returns the standard path found that contains the subPath
    //! local paths have higher priority by default
    static QString standardPath(QString subPath, bool localFirst = true);
    //! returns all application data standard paths
    //! local paths have higher priority by default
    static QStringList standardPaths(bool localfirst = true);
    static QStringList standardPathsFor(QString subPath, bool localfirst = true);   

    //! check if this layout exists already in the latte directory
    static bool layoutExists(QString layoutName);
    //! imports the specific layout and return the new layout name.
    //! if the function didn't succeed returns an empty string
    static QString importLayoutHelper(const QString &fileName, const QString &suggestedName = QString());

    //! returns the file path of a layout either existing or not
    static QString layoutUserFilePath(QString layoutName);
    //! returns the layouts user directory
    static QString layoutUserDir();
    //! returns the system path for latte shell data
    static QString systemShellDataPath();

    static QString nameOfConfigFile(const QString &fileName);
    static QString uniqueLayoutName(QString name);

    static bool hasViewTemplate(const QString &name);
    static QString layoutTemplateSystemFilePath(const QString &name);

    static QStringList availableLayouts();
    static QStringList availableLayoutTemplates();
    static QStringList availableViewTemplates();
    //! it checks the linked file if there are Containments in it that belong
    //! to Original Layouts and moves them accordingly. This is used mainly on
    //! startup and if such state occurs, it basically means that the app didn't
    //! close correctly, e.g. there was a crash.
    static QStringList checkRepairMultipleLayoutsLinkedFile();

    static Latte::MultipleLayouts::Status multipleLayoutsStatus();
    static void setMultipleLayoutsStatus(const Latte::MultipleLayouts::Status &status);

signals:
    void newLayoutAdded(const QString &path);

private:
    //! checks if this old layout can be imported. If it can it returns
    //! the new layout path and an empty string if it cant
    QString layoutCanBeImported(QString oldAppletsPath, QString newName, QString exportDirectory = QString());

    QTemporaryDir m_storageTmpDir;

    Layouts::Manager *m_manager;
};

}
}

#endif // IMPORTER_H
