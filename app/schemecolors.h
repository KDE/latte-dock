/*
 * Copyright 2018  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef SCHEMECOLORS_H
#define SCHEMECOLORS_H

#include <QObject>
#include <QColor>

namespace Latte {

class SchemeColors: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor NOTIFY colorsChanged)

public:
    enum ColorsSubgroup
    {
        Active = 0,
        Inactive = 1
    };
    Q_ENUM(ColorsSubgroup);

    SchemeColors(QObject *parent, QString scheme);
    ~SchemeColors() override;

    QString schemeName();
    QString schemeFile();

    QColor backgroundColor() const;
    QColor foregroundColor() const;

    SchemeColors::ColorsSubgroup subgroup() const;
    void setSubgroup(SchemeColors::ColorsSubgroup subgroup);

    static QString possibleSchemeFile(QString scheme);

signals:
    void colorsChanged();

private slots:
    void updateScheme();

private:
    QString m_schemeName;
    QString m_schemeFile;

    QColor m_activeBackgroundColor;
    QColor m_activeForegroundColor;

    QColor m_inactiveBackgroundColor;
    QColor m_inactiveForegroundColor;

    ColorsSubgroup m_subgroup{SchemeColors::Active};
};

}

#endif
