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

#ifndef SETTINGSGENERICHANDLER_H
#define SETTINGSGENERICHANDLER_H

//! Qt
#include <QAction>
#include <QObject>
#include <QPushButton>

namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class Generic : public QObject
{
    Q_OBJECT
public:
    static constexpr const char* TWINENABLED = "Enabled";
    static constexpr const char* TWINVISIBLE = "Visible";
    static constexpr const char* TWINCHECKED = "Checked";

    Generic(QObject *parent);

protected:
    void setTwinProperty(QAction *action, const QString &property, QVariant value);
    void connectActionWithButton(QPushButton *button, QAction *action);

protected:
    //! Twin Actions bind QAction* behavior with QPushButton*
    //! for simplicity reasons
    QHash<QAction *, QPushButton *> m_twinActions;

};

}
}
}

#endif
