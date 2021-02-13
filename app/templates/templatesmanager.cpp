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
#include "../layout/abstractlayout.h"
#include "../layout/centrallayout.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../layouts/storage.h"
#include "../tools/commontools.h"
#include "../view/view.h"

// Qt
#include <QDir>

// KDE
#include <KDirWatch>
#include <KLocalizedString>

namespace Latte {
namespace Templates {

Manager::Manager(Latte::Corona *corona)
    : QObject(corona),
      m_corona(corona)
{
    KDirWatch::self()->addDir(Latte::configPath() + "/latte/templates", KDirWatch::WatchFiles);
    connect(KDirWatch::self(), &KDirWatch::created, this, &Manager::onCustomTemplatesCountChanged);
    connect(KDirWatch::self(), &KDirWatch::deleted, this, &Manager::onCustomTemplatesCountChanged);
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &Manager::onCustomTemplatesCountChanged);
}

Manager::~Manager()
{
}

void Manager::init()
{
    connect(this, &Manager::viewTemplatesChanged, m_corona->layoutsManager(), &Latte::Layouts::Manager::viewTemplatesChanged);

    initLayoutTemplates();
    initViewTemplates();
}

void Manager::initLayoutTemplates()
{
    m_layoutTemplates.clear();
    initLayoutTemplates(m_corona->kPackage().filePath("templates"));
    initLayoutTemplates(Latte::configPath() + "/latte/templates");
    emit layoutTemplatesChanged();
}

void Manager::initViewTemplates()
{
    m_viewTemplates.clear();
    initViewTemplates(m_corona->kPackage().filePath("templates"));
    initViewTemplates(Latte::configPath() + "/latte/templates");
    emit viewTemplatesChanged();
}

void Manager::initLayoutTemplates(const QString &path)
{
    QDir templatesDir(path);
    QStringList filter;
    filter.append(QString("*.layout.latte"));
    QStringList templates = templatesDir.entryList(filter, QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    for (int i=0; i<templates.count(); ++i) {
        QString templatePath = templatesDir.path() + "/" + templates[i];
        if (!m_layoutTemplates.containsId(templatePath)) {
            CentralLayout layouttemplate(this, templatePath);

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

void Manager::initViewTemplates(const QString &path)
{
    QDir templatesDir(path);
    QStringList filter;
    filter.append(QString("*.view.latte"));
    QStringList templates = templatesDir.entryList(filter, QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    for (int i=0; i<templates.count(); ++i) {
        QString templatePath = templatesDir.path() + "/" + templates[i];

        if (!m_viewTemplates.containsId(templatePath)) {
            Data::Generic vdata;
            vdata.id = templatePath;
            vdata.name = QFileInfo(templatePath).baseName();

            m_viewTemplates << vdata;
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
             && m_layoutTemplates[i].name != Layout::MULTIPLELAYOUTSHIDDENNAME) {
            templates << m_layoutTemplates[i];
        }
    }

    return templates;
}

Data::GenericTable<Data::Generic> Manager::viewTemplates()
{
    return m_viewTemplates;
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

    QString newLayoutPath = Layouts::Importer::layoutUserFilePath(layoutName);

    Data::Layout dlayout = layoutTemplateForName(layoutTemplate);
    QFile(dlayout.id).copy(newLayoutPath);
    qDebug() << "adding layout : " << layoutName << " based on layout template:" << layoutTemplate;

    emit newLayoutAdded(newLayoutPath);

    return newLayoutPath;
}

bool Manager::exportTemplate(const QString &originFile, const QString &destinationFile, const Data::AppletsTable &approvedApplets)
{
    return Latte::Layouts::Storage::self()->exportTemplate(originFile, destinationFile, approvedApplets);
}

bool Manager::exportTemplate(const Latte::View *view, const QString &destinationFile, const Data::AppletsTable &approvedApplets)
{
    return Latte::Layouts::Storage::self()->exportTemplate(view->layout(), view->containment(), destinationFile, approvedApplets);
}

void Manager::onCustomTemplatesCountChanged(const QString &file)
{
    if (file.startsWith(Latte::configPath() + "/latte/templates")) {
        if (file.endsWith(".layout.latte")) {
            initLayoutTemplates();
        } else if (file.endsWith(".view.latte")) {
            initViewTemplates();
        }
    }
}

void Manager::importSystemLayouts()
{
    for (int i=0; i<m_layoutTemplates.rowCount(); ++i) {
        if (m_layoutTemplates[i].isSystemTemplate()) {
            QString userLayoutPath = Layouts::Importer::layoutUserFilePath(m_layoutTemplates[i].name);

            if (!QFile(userLayoutPath).exists()) {
                QFile(m_layoutTemplates[i].id).copy(userLayoutPath);
                qDebug() << "adding layout : " << userLayoutPath << " based on layout template:" << m_layoutTemplates[i].name;
            }
        }
    }
}

QString Manager::proposedTemplateAbsolutePath(QString templateFilename)
{
    QString tempfilename = templateFilename;

    if (tempfilename.endsWith(".layout.latte")) {
        QString clearedname = QFileInfo(tempfilename).baseName();
        tempfilename = uniqueLayoutTemplateName(clearedname) + ".layout.latte";
    } else if (tempfilename.endsWith(".view.latte")) {
        QString clearedname = QFileInfo(tempfilename).baseName();
        tempfilename = uniqueViewTemplateName(clearedname) + ".view.latte";
    }

    return QString(Latte::configPath() + "/latte/templates/" + tempfilename);
}


bool Manager::layoutTemplateExists(const QString &templateName) const
{
    return m_layoutTemplates.containsName(templateName);
}

bool Manager::viewTemplateExists(const QString &templateName) const
{
    return m_viewTemplates.containsName(templateName);
}

QString Manager::uniqueLayoutTemplateName(QString name) const
{
    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (layoutTemplateExists(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (layoutTemplateExists(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}

QString Manager::uniqueViewTemplateName(QString name) const
{
    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (viewTemplateExists(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (viewTemplateExists(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}


//! it is used in order to provide translations for system templates
void Manager::exposeTranslatedTemplateNames()
{
    i18nc("default layout template name", "Default");
    i18nc("empty layout template name", "Empty");
}

}
}
