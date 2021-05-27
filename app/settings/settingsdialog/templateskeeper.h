/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SETTINGSPARTTEMPLATESKEEPER_H
#define SETTINGSPARTTEMPLATESKEEPER_H

// local
#include "../../data/viewdata.h"
#include "../../data/viewstable.h"

// Qt
#include <QObject>

namespace Latte {
class CentralLayout;
class Corona;
namespace Settings {
namespace Controller {
class Layouts;
}
}
}

namespace Latte {
namespace Settings {
namespace Part {

class TemplatesKeeper : public QObject
{
    Q_OBJECT

public:
    explicit TemplatesKeeper(Settings::Controller::Layouts *parent, Latte::Corona *corona);
    ~TemplatesKeeper();

    QString storedView(const QString &layoutCurrentId, const QString &viewId);

    bool hasClipboardContents() const;

    Latte::Data::ViewsTable clipboardContents() const;
    void setClipboardContents(const Latte::Data::ViewsTable &views);

public slots:
    void clear();

signals:
    void clipboardContentsChanged();

private:
    QString viewKeeperId(const QString &layoutCurrentId, const QString &viewId);

private:
    Latte::Data::ViewsTable m_storedViews;
    Latte::Data::ViewsTable m_clipboardViews;

    Latte::Corona *m_corona{nullptr};
    Settings::Controller::Layouts *m_layoutsController{nullptr};

    QList<CentralLayout *> m_garbageLayouts;
};

}
}
}

#endif
