/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ORIGINALVIEW_H
#define ORIGINALVIEW_H

// local
#include <coretypes.h>
#include "view.h"

// Qt
#include <QList>

namespace Latte {

class ClonedView;

class OriginalView : public View
{
    Q_OBJECT
    Q_PROPERTY(Latte::Types::ScreensGroup screensGroup READ screensGroup NOTIFY screensGroupChanged)

public:
    OriginalView(Plasma::Corona *corona, QScreen *targetScreen = nullptr, bool byPassX11WM = false);
    ~OriginalView();

    bool isOriginal() const override;
    bool isCloned() const override;
    bool isSingle() const override;

    int clonesCount() const;

    int expectedScreenIdFromScreenGroup(const Latte::Types::ScreensGroup &nextScreensGroup) const;

    Latte::Types::ScreensGroup screensGroup() const override;
    void setScreensGroup(const Latte::Types::ScreensGroup &group);

    void addClone(Latte::ClonedView *view);
    void setNextLocationForClones(const QString layoutName, int edge, int alignment);
    void addApplet(const QString &pluginId, const int &excludecloneid);
    void addApplet(QObject *mimedata, const int &x, const int &y, const int &excludecloneid);

    void reconsiderScreen() override;

public slots:
    void cleanClones();

signals:
    void screensGroupChanged();    

private slots:    
    void syncClonesToScreens();

    void restoreConfig();
    void saveConfig();

private:
    void createClone(int screenId);
    void removeClone(Latte::ClonedView *view);

private:
    Latte::Types::ScreensGroup m_screensGroup{Latte::Types::SingleScreenGroup};
    QList<Latte::ClonedView *> m_clones;

    QList<int> m_waitingCreation;
};

}

#endif
