/*
    SPDX-FileCopyrightText: 2019 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

    void blockModifierTracking(Qt::Key key);
    void unblockModifierTracking(Qt::Key key);

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

    //! modifiers that the user does not want to track anymore
    QList<Qt::Key> m_blockedModifiers;

    //! keep a record for modifiers
    QHash<Qt::Key, bool> m_pressed;

};

}
}

#endif
