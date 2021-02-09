/*
 * Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COLORSMODEL_H
#define COLROSMODEL_H

// local
#include "../../lattecorona.h"
#include "../../data/layoutcolordata.h"

// Qt
#include <QAbstractTableModel>
#include <QModelIndex>


namespace Latte {
namespace Settings {
namespace Model {

class Colors : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ColorsRoles
    {
        IDROLE = Qt::UserRole + 1,
        NAMEROLE,
        PATHROLE,
        TEXTCOLORROLE
    };

    explicit Colors(QObject *parent, Latte::Corona *corona);
    ~Colors();

    int rowCount() const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    int row(const QString &id);

    QString colorPath(const QString &color);

    QVariant data(const QModelIndex &index, int role) const override;

private:
    void init();
    void add(const QString &newid, const QString &newname, const QString &newpath, const QString &newtextcolor);  

private:
    QString m_colorsPath;

    QList<Latte::Data::LayoutColor> m_colorsTable;

    Latte::Corona *m_corona{nullptr};
};

}
}
}

#endif
