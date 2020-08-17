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

#ifndef LAYOUTSTORAGE_H
#define LAYOUTSTORAGE_H

#include "genericlayout.h"

// Qt
#include <QObject>

namespace Plasma{
class Containment;
}

namespace Latte {
namespace Layout {

class Storage : public QObject
{
    Q_OBJECT

public:
    Storage(GenericLayout *parent);
    ~Storage() override;

    bool layoutIsBroken(QStringList &errors) const;

    void copyView(Plasma::Containment *containment);
    void syncToLayoutFile(bool removeLayoutId);

    void setStorageTmpDir(const QString &tmpDir);

    //! Functions used from Layout Reports
    //! [containment id, list<systrays ids>], list<systrays ids>, list[systrays ids]
    void systraysInformation(QHash<int, QList<int>> &systrays, QList<int> &assignedSystrays, QList<int> &orphanSystrays);
    //! list<screens ids>
    QList<int> viewsScreens();
    //! list<ViewData>
    QList<ViewData> viewsData(const QHash<int, QList<int>> &systrays);

private:
    //! STORAGE !////
    QString availableId(QStringList all, QStringList assigned, int base);
    //! provides a new file path based the provided file. The new file
    //! has updated ids for containments and applets based on the corona
    //! loaded ones
    QString newUniqueIdsLayoutFromFile(QString file);
    //! imports a layout file and returns the containments for the docks
    QList<Plasma::Containment *> importLayoutFile(QString file);

private:
    GenericLayout *m_layout;

    QString m_storageTmpDir;
};

}
}

#endif
