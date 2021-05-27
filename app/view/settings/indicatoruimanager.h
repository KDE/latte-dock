/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INDICATORUIMANAGER_H
#define INDICATORUIMANAGER_H

//Qt
#include <QList>
#include <QObject>
#include <QQuickItem>
#include <QPointer>

namespace KDeclarative
{
class QmlObjectSharedEngine;
}

namespace Latte {
class View;
}

namespace Latte {
namespace ViewPart {
class PrimaryConfigView;
}
}

namespace Latte {
namespace ViewPart {
namespace Config {

struct IndicatorUiData
{
    QString type;
    QString pluginPath;
    QString name;
    QPointer<Latte::View> view;
    QPointer<KDeclarative::QmlObjectSharedEngine> ui;
};

class IndicatorUiManager : public QObject
{
    Q_OBJECT

public:
    IndicatorUiManager(ViewPart::PrimaryConfigView *parent);
    ~IndicatorUiManager() override;

public slots:
    Q_INVOKABLE void addIndicator();
    Q_INVOKABLE void downloadIndicator();
    Q_INVOKABLE void removeIndicator(QString pluginId);
    Q_INVOKABLE void setParentItem(QQuickItem *parentItem);
    Q_INVOKABLE void ui(const QString &type, Latte::View *view);

    Q_INVOKABLE int index(const QString &type);
private:
    bool contains(const QString &type);

    void hideAllUi();
    void showNextIndicator();

private:
    QQuickItem *m_parentItem{nullptr};
    PrimaryConfigView *m_primary{nullptr};

    QList<IndicatorUiData> m_uidata;

};

}
}
}

#endif

