/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
    bool istranslated = (m_corona->kPackage().filePath("templates") == path);

    QDir templatesDir(path);
    QStringList filter;
    filter.append(QString("*.view.latte"));
    QStringList templates = templatesDir.entryList(filter, QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    for (int i=0; i<templates.count(); ++i) {
        QString templatePath = templatesDir.path() + "/" + templates[i];

        if (!m_viewTemplates.containsId(templatePath)) {
            Data::Generic vdata;
            vdata.id = templatePath;
            QString tname = QFileInfo(templatePath).baseName();

            if (istranslated) {
                QByteArray tnamechars = tname.toUtf8();
                vdata.name = i18nc("view template name", tnamechars);
            } else {
                vdata.name = tname;
            }

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

Data::LayoutsTable Manager::layoutTemplates()
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

Data::GenericBasicTable Manager::viewTemplates()
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
        QString clearedname = tempfilename.chopped(QString(".layout.latte").size());
        tempfilename = uniqueLayoutTemplateName(clearedname) + ".layout.latte";
    } else if (tempfilename.endsWith(".view.latte")) {
        QString clearedname = tempfilename.chopped(QString(".view.latte").size());
        tempfilename = uniqueViewTemplateName(clearedname) + ".view.latte";
    }

    return QString(Latte::configPath() + "/latte/templates/" + tempfilename);
}

bool Manager::hasCustomLayoutTemplate(const QString &templateName) const
{
    for (int i=0; i<m_layoutTemplates.rowCount(); ++i) {
        if (m_layoutTemplates[i].name == templateName && !m_layoutTemplates[i].isSystemTemplate()) {
            return true;
        }
    }

    return false;
}

bool Manager::hasLayoutTemplate(const QString &templateName) const
{
    return m_layoutTemplates.containsName(templateName);
}

bool Manager::hasViewTemplate(const QString &templateName) const
{
    return m_viewTemplates.containsName(templateName);
}

QString Manager::viewTemplateFilePath(const QString templateName) const
{
    if (m_viewTemplates.containsName(templateName)) {
        return m_viewTemplates.idForName(templateName);
    }

    return QString();
}

void Manager::installCustomLayoutTemplate(const QString &templateFilePath)
{
    if (!templateFilePath.endsWith(".layout.latte")) {
        return;
    }

    QString layoutName = QFileInfo(templateFilePath).baseName();

    QString destinationFilePath = Latte::configPath() + "/latte/templates/" + layoutName + ".layout.latte";

    if (hasCustomLayoutTemplate(layoutName)) {
        QFile(destinationFilePath).remove();
    }

    QFile(templateFilePath).copy(destinationFilePath);
}

QString Manager::uniqueLayoutTemplateName(QString name) const
{
    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (hasLayoutTemplate(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (hasLayoutTemplate(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}

QString Manager::uniqueViewTemplateName(QString name) const
{
    int pos_ = name.lastIndexOf(QRegExp(QString(" - [0-9]+")));

    if (hasViewTemplate(name) && pos_ > 0) {
        name = name.left(pos_);
    }

    int i = 2;

    QString namePart = name;

    while (hasViewTemplate(name)) {
        name = namePart + " - " + QString::number(i);
        i++;
    }

    return name;
}

QString Manager::templateName(const QString &filePath)
{
    int lastSlash = filePath.lastIndexOf("/");
    QString tempFilePath = filePath;
    QString templatename = tempFilePath.remove(0, lastSlash + 1);

    QString extension(".layout.latte");
    int ext = templatename.lastIndexOf(extension);
    if (ext>0) {
        templatename = templatename.remove(ext, extension.size());
    } else {
        extension = ".view.latte";
        ext = templatename.lastIndexOf(extension);
        templatename = templatename.remove(ext,extension.size());
    }

    return templatename;
}

//! it is used in order to provide translations for system templates
void Manager::exposeTranslatedTemplateNames()
{
    //! layout templates default names
    i18nc("default layout template name", "Default");
    i18nc("empty layout template name", "Empty");

    //! dock/panel templates default names
    i18nc("view template name", "Default Dock");
    i18nc("view template name", "Default Panel");
    i18nc("view template name", "Empty Panel");
}

}
}
