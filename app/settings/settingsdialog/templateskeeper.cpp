/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
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

Latte::Data::ViewsTable TemplatesKeeper::clipboardContents() const
{
    return m_clipboardViews;
}

void TemplatesKeeper::setClipboardContents(const Latte::Data::ViewsTable &views)
{
    m_clipboardViews.clear();
    m_clipboardViews = views;
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
