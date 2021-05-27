/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// local
#include "modifiertracker.h"

// Qt
#include <QDebug>
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
    m_metaPressedTimer.setSingleShot(true);
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
    if (m_blockedModifiers.contains(key)) {
        return false;
    }

    return (key == Qt::Key_Super_L || key == Qt::Key_Super_R || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Shift);
}

void ModifierTracker::blockModifierTracking(Qt::Key key)
{
    if (!m_blockedModifiers.contains(key)) {
        m_blockedModifiers.append(key);
    }
}

void ModifierTracker::unblockModifierTracking(Qt::Key key)
{
    if (m_blockedModifiers.contains(key)) {
        m_blockedModifiers.removeAll(key);
    }
}

bool ModifierTracker::noModifierPressed()
{
    for (const Qt::Key &modifier : m_pressed.keys()) {
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
    for (const Qt::Key &modifier : m_pressed.keys()) {
        if ( (modifier != key && m_pressed[modifier])
             || (modifier == key && !m_pressed[modifier]) ) {
            return false;
        }
    }

    return true;
}

Qt::Key ModifierTracker::normalizeKey(Qt::Key key)
{
    return ((key == Qt::Key_Super_L || key == Qt::Key_Super_R) ? Qt::Key_Super_L : key);
}

void ModifierTracker::cancelMetaPressed()
{
    m_metaPressedTimer.stop();
}


}
}
