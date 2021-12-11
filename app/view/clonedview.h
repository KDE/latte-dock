/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLONEDVIEW_H
#define CLONEDVIEW_H

// local
#include <coretypes.h>
#include "originalview.h"
#include "indicator/indicator.h"
#include "view.h"

namespace Latte {

class ClonedView : public View
{
    Q_OBJECT

public:
    static const int ERRORAPPLETID = -999;

    ClonedView(Plasma::Corona *corona, Latte::OriginalView *originalView, QScreen *targetScreen = nullptr, bool byPassX11WM = false);
    ~ClonedView();

    bool isOriginal() const override;
    bool isCloned() const override;
    bool isSingle() const override;

    bool isPreferredForShortcuts() const override;
    int groupId() const override;

    Latte::Types::ScreensGroup screensGroup() const override;

    ViewPart::Indicator *indicator() const override;
    Latte::Data::View data() const override;

protected:
    void showConfigurationInterface(Plasma::Applet *applet) override;

private slots:
    void initSync();
    void onOriginalAppletConfigPropertyChanged(const int &id, const QString &key, const QVariant &value);
    void onOriginalAppletInScheduledDestructionChanged(const int &id, const bool &enabled);
    void onOriginalAppletRemoved(const int &id);
    void onOriginalAppletsOrderChanged();
    void onOriginalAppletsInLockedZoomChanged(const QList<int> &originalapplets);
    void onOriginalAppletsDisabledColoringChanged(const QList<int> &originalapplets);

    void updateContainmentConfigProperty(const QString &key, const QVariant &value);
    void updateOriginalAppletConfigProperty(const int &clonedid, const QString &key, const QVariant &value);

    void updateAppletIdsHash();
private:
    bool isTranslatableToClonesOrder(const QList<int> &originalOrder);

    bool hasOriginalAppletId(const int &clonedid);
    int originalAppletId(const int &clonedid);

    QList<int> translateToClonesOrder(const QList<int> &originalIds);

private:
    static QStringList CONTAINMENTMANUALSYNCEDPROPERTIES;

    QPointer<Latte::OriginalView> m_originalView;
    QHash<int, int> m_currentAppletIds;
};

}

#endif
