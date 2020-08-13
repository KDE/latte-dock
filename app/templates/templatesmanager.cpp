/*
*  Copyright 2020  Michail Vourlakos <mvourlakos@gmail.com>
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

#include "templatesmanager.h"

// local
#include "../layout/centrallayout.h"
#include "../layouts/importer.h"

// Qt
#include <QDir>

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Templates {

Manager::Manager(Latte::Corona *corona)
    : QObject(corona),
      m_corona(corona)
{
}

Manager::~Manager()
{
}

void Manager::init()
{
    QDir systemTemplatesDir(m_corona->kPackage().filePath("templates"));
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList systemLayoutTemplates = systemTemplatesDir.entryList(filter, QDir::Files | QDir::NoSymLinks);

    for (int i=0; i<systemLayoutTemplates.count(); ++i) {
        QString systemTemplatePath = systemTemplatesDir.path() + "/" + systemLayoutTemplates[i];
        if (!m_layoutTemplates.containsId(systemTemplatePath)) {
            CentralLayout layouttemplate(this, systemTemplatePath);

            Data::Layout tdata = layouttemplate.data();
            tdata.isTemplate = true;

            if (tdata.name == DEFAULTLAYOUTTEMPLATENAME || tdata.name == EMPTYLAYOUTTEMPLATENAME) {
                QByteArray templateNameChars = tdata.name.toUtf8();
                tdata.name = i18n(templateNameChars);
            }

            m_layoutTemplates << tdata;
        }
    }
}

Data::Layout Manager::layoutTemplateForName(const QString &layoutName)
{
    if (m_layoutTemplates.containsName(layoutName)) {
        QString layoutid = m_layoutTemplates.idForName(layoutName);
        return m_layoutTemplates[layoutid];
    }

    return Data::Layout();
}

Data::LayoutsTable Manager::systemLayoutTemplates()
{
    Data::LayoutsTable templates;

    QString id = m_layoutTemplates.idForName(i18n(DEFAULTLAYOUTTEMPLATENAME));
    templates << m_layoutTemplates[id];
    id = m_layoutTemplates.idForName(i18n(EMPTYLAYOUTTEMPLATENAME));
    templates << m_layoutTemplates[id];

    for (int i=0; i<m_layoutTemplates.rowCount(); ++i) {
        if ( m_layoutTemplates[i].name != i18n(DEFAULTLAYOUTTEMPLATENAME)
             && m_layoutTemplates[i].name != i18n(EMPTYLAYOUTTEMPLATENAME)
             && m_layoutTemplates[i].name != MULTIPLELAYOUTSTEMPLATENAME) {
            templates << m_layoutTemplates[i];
        }
    }

    return templates;
}

QString Manager::newLayout(QString layoutName, QString layoutTemplate)
{
    if (!m_layoutTemplates.containsName(layoutTemplate)) {
        return QString();
    }

    if (layoutName.isEmpty()) {
        layoutName = Layouts::Importer::uniqueLayoutName(layoutTemplate);
    } else {
        layoutName = Layouts::Importer::uniqueLayoutName(layoutName);
    }

    QString newLayoutPath = QDir::homePath() + "/.config/latte/" + layoutName + ".layout.latte";

    Data::Layout dlayout = layoutTemplateForName(layoutTemplate);
    QFile(dlayout.id).copy(newLayoutPath);
    qDebug() << "adding layout : " << layoutName << " based on layout template:" << layoutTemplate;

    emit newLayoutAdded(newLayoutPath);

    return newLayoutPath;
}


//! it is used just in order to provide translations for the presets
void Manager::exposeTranslatedTemplateNames()
{
    i18nc("default layout template name", "Default");
    i18n("empty layout template name", "Empty");
}

}
}
