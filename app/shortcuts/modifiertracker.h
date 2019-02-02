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

#ifndef MODIFIERTRACKER_H
#define MODIFIERTRACKER_H

// Qt
#include <QHash>
#include <QObject>
#include <QTimer>

// KDE
#include <KModifierKeyInfo>

namespace Latte {
namespace ShortcutsPart {

class ModifierTracker: public QObject {
    Q_OBJECT

public:
    ModifierTracker(QObject *parent);
    ~ModifierTracker() override;

    //! cancel meta is pressed delayer
    void cancelMetaPressed();

    //! none of tracked modifiers is pressed
    bool noModifierPressed();

    //! at least one of the modifiers from KeySequence is pressed
    bool sequenceModifierPressed(const QKeySequence &seq);

    //! only <key> is pressed and no other modifier
    bool singleModifierPressed(Qt::Key key);

signals:
    void metaModifierPressed();
    void modifiersChanged();

private:
    void init();

    //! <key> modifier is tracked for changes
    bool modifierIsTracked(Qt::Key key);

    //! adjust key in more general values, e.g. Super_L and Super_R both return Super_L
    Qt::Key normalizeKey(Qt::Key key);

private:
    KModifierKeyInfo m_modifierKeyInfo;
    QTimer m_metaPressedTimer;

    //! keep a record for modifiers
    QHash<Qt::Key, bool> m_pressed;

};

}
}

#endif
