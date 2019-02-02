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

#include "modifiertracker.h"

#include <QKeySequence>

namespace Latte {
namespace ShortcutsPart {

ModifierTracker::ModifierTracker(QObject *parent)
    : QObject(parent)
{
    init();
}

ModifierTracker::~ModifierTracker()
{
}

void ModifierTracker::init()
{
    m_metaPressedTimer.setInterval(700);

    m_pressed[Qt::Key_Super_L] = false;
    m_pressed[Qt::Key_Control] = false;
    m_pressed[Qt::Key_Alt] = false;
    m_pressed[Qt::Key_Shift] = false;

    connect(&m_metaPressedTimer, &QTimer::timeout, this, &ModifierTracker::metaModifierPressed);

    connect(&m_modifierKeyInfo, &KModifierKeyInfo::keyPressed, this, [&](Qt::Key key, bool state) {
        Qt::Key nKey = normalizeKey(key);
        //! ignore modifiers that we do not take into account
        if (!modifierIsTracked(nKey)) {
            return;
        }

        m_pressed[nKey] = state;

        if (nKey == Qt::Key_Super_L) {
            bool singleKey{singleModifierPressed(Qt::Key_Super_L)};
            if (state && singleKey) {
                m_metaPressedTimer.start();
            } else if (!state || !singleKey) {
                cancelMetaPressed();
            }
        } else {
            cancelMetaPressed();
        }

        emit modifiersChanged();
    });
}


bool ModifierTracker::modifierIsTracked(Qt::Key key)
{
    return(key == Qt::Key_Super_L || key == Qt::Key_Super_R || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Shift);
}

bool ModifierTracker::noModifierPressed()
{
    foreach(Qt::Key modifier, m_pressed.keys()) {
        if ( m_pressed[modifier]) {
            return false;
        }
    }

    return true;
}

bool ModifierTracker::sequenceModifierPressed(const QKeySequence &seq)
{
    if (seq.isEmpty()) {
        return false;
    }

    int mod = seq[seq.count() - 1] & Qt::KeyboardModifierMask;

    if ( ((mod & Qt::SHIFT) && m_pressed[Qt::Key_Shift])
         || ((mod & Qt::CTRL) && m_pressed[Qt::Key_Control])
         || ((mod & Qt::ALT) && m_pressed[Qt::Key_Alt])
         || ((mod & Qt::META) && m_pressed[Qt::Key_Super_L])) {
        return true;
    }

    return false;
}

bool ModifierTracker::singleModifierPressed(Qt::Key key)
{
    foreach(Qt::Key modifier, m_pressed.keys()) {
        if ( (modifier != key && m_pressed[modifier])
             || (modifier == key && !m_pressed[modifier]) ) {
            return false;
        }
    }

    return true;
}

Qt::Key ModifierTracker::normalizeKey(Qt::Key key)
{
    return key == Qt::Key_Super_L || key == key == Qt::Key_Super_R ? Qt::Key_Super_L : key;
}

void ModifierTracker::cancelMetaPressed()
{
    m_metaPressedTimer.stop();
}


}
}
