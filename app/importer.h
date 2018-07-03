/*
*  Copyright 2017  Smith AR <audoban@openmailbox.org>
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

#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>

namespace Latte {
class LayoutManager;
}

namespace Latte {

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

    static Importer::LatteFileVersion fileVersion(QString file);

    static bool importHelper(QString fileName);
    //! check if this layout exists already in the latte directory
    static bool layoutExists(QString layoutName);
    //! imports the specific layout and return the new layout name.
    //! if the function didnt succeed return an empty string
    static QString importLayoutHelper(QString fileName);

    //! return the file path of a layout either existing or not
    static QString layoutFilePath(QString layoutName);
    static QString nameOfConfigFile(const QString &fileName);
    static QString uniqueLayoutName(QString name);

    static QStringList availableLayouts();
    //! it checks the linked file if there are Containments in it that belong
    //! to Original Layouts and moves them accordingly. This is used mainly on
    //! startup and if such state occurs, it basically means that the app didnt
    //! close correctly, e.g. there was a crash.
    static QStringList checkRepairMultipleLayoutsLinkedFile();

private:
    //! checks if this old layout can be imported. If it can it returns
    //! the new layout path and an empty string if it cant
    QString layoutCanBeImported(QString oldAppletsPath, QString newName, QString exportDirectory = QString());

    LayoutManager *m_manager;
};

}

#endif // IMPORTER_H
