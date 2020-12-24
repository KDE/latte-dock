/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#ifndef PADDING_H
#define PADDING_H

// Qt
#include <QObject>


namespace Latte {
namespace ViewPart {

class Padding: public QObject
{
    Q_OBJECT

    Q_PROPERTY(int top READ top WRITE setTop NOTIFY paddingsChanged)
    Q_PROPERTY(int bottom READ bottom WRITE setBottom NOTIFY paddingsChanged)
    Q_PROPERTY(int left READ left WRITE setLeft NOTIFY paddingsChanged)
    Q_PROPERTY(int right READ right WRITE setRight NOTIFY paddingsChanged)

public:
    Padding(QObject *parent);
    virtual ~Padding();

    bool isEmpty() const;

    int top() const;
    void setTop(int toppad);

    int bottom() const;
    void setBottom(int bottompad);

    int left() const;
    void setLeft(int leftpad);

    int right() const;
    void setRight(int rightpad);

signals:
    void paddingsChanged();

private:
    int m_left{0};
    int m_right{0};
    int m_top{0};
    int m_bottom{0};
};

}
}

#endif
