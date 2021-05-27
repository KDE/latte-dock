/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "templateskeeper.h"

// local
#include "layoutscontroller.h"
#include "../../lattecorona.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"

namespace Latte {
namespace Settings {
namespace Part {

TemplatesKeeper::TemplatesKeeper(Settings::Controller::Layouts *parent, Latte::Corona *corona)
    : QObject(parent),
      m_corona(corona),
      m_layoutsController(parent)
{
}

TemplatesKeeper::~TemplatesKeeper()
{
    clear();
}

bool TemplatesKeeper::hasClipboardContents() const
{
    return (m_clipboardViews.rowCount() > 0);
}

Latte::Data::ViewsTable TemplatesKeeper::clipboardContents() const
{
    return m_clipboardViews;
}

void TemplatesKeeper::setClipboardContents(const Latte::Data::ViewsTable &views)
{
    if (m_clipboardViews == views) {
        return;
    }

    m_clipboardViews.clear();
    m_clipboardViews = views;

    emit clipboardContentsChanged();
}

void TemplatesKeeper::clear()
{
    qDeleteAll(m_garbageLayouts);

    m_garbageLayouts.clear();
    m_storedViews.clear();
    m_clipboardViews.clear();
}

QString TemplatesKeeper::viewKeeperId(const QString &layoutCurrentId, const QString &viewId)
{
    return QString(layoutCurrentId + "#" + viewId);

}

QString TemplatesKeeper::storedView(const QString &layoutCurrentId, const QString &viewId)
{
    QString keeperid = viewKeeperId(layoutCurrentId, viewId);

    if (m_storedViews.containsId(keeperid)) {
        return m_storedViews[keeperid].originFile();
    }

    Latte::Data::Layout originallayout = m_layoutsController->originalData(layoutCurrentId);
    Latte::Data::Layout currentlayout = m_layoutsController->currentData(layoutCurrentId);
    Latte::CentralLayout *centralActive =  m_layoutsController->isLayoutOriginal(layoutCurrentId) ? m_corona->layoutsManager()->synchronizer()->centralLayout(originallayout.name) : nullptr;
    Latte::CentralLayout *central = centralActive ? centralActive : new Latte::CentralLayout(this, layoutCurrentId);

    if (!centralActive) {
        m_garbageLayouts << central;
    }

    if (!central->containsView(viewId.toInt())) {
        return QString();
    }

    QString storedviewpath = central->storedView(viewId.toInt());

    Latte::Data::View storedview;
    storedview.id = keeperid;
    storedview.isActive = false;
    storedview.setState(Data::View::OriginFromViewTemplate, storedviewpath, layoutCurrentId, viewId);
    m_storedViews << storedview;

    return storedviewpath;
}


}
}
}
