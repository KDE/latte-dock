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

#ifndef TEMPLATESMANAGER_H
#define TEMPLATESMANAGER_H

// local
#include "../lattecorona.h"
#include "../data/appletdata.h"
#include "../data/layoutdata.h"
#include "../data/layoutstable.h"
#include "../data/genericdata.h"
#include "../data/generictable.h"

// Qt
#include <QObject>

// KDE
#include <KLocalizedString>

namespace Latte {
class Corona;
class View;
}

namespace Latte {
namespace Templates {

const char DEFAULTLAYOUTTEMPLATENAME[] = "Default";
const char EMPTYLAYOUTTEMPLATENAME[] = "Empty";

class Manager : public QObject
{
    Q_OBJECT

public:
    Manager(Latte::Corona *corona = nullptr);
    ~Manager() override;

    Latte::Corona *corona();
    void init();

    bool hasCustomLayoutTemplate(const QString &templateName) const;
    bool hasLayoutTemplate(const QString &templateName) const;
    bool hasViewTemplate(const QString &templateName) const;

    bool exportTemplate(const QString &originFile, const QString &destinationFile, const Data::AppletsTable &approvedApplets);
    bool exportTemplate(const Latte::View *view, const QString &destinationFile, const Data::AppletsTable &approvedApplets);

    Data::Layout layoutTemplateForName(const QString &layoutName);

    Data::LayoutsTable layoutTemplates();
    Data::GenericTable<Data::Generic> viewTemplates();

    //! creates a new layout with layoutName based on specific layout template and returns the new layout path
    QString newLayout(QString layoutName, QString layoutTemplate = i18n(DEFAULTLAYOUTTEMPLATENAME));

    QString proposedTemplateAbsolutePath(QString templateFilename);

    void importSystemLayouts();
    void installCustomLayoutTemplate(const QString &templateFilePath);

signals:
    void newLayoutAdded(const QString &path);
    void layoutTemplatesChanged();
    void viewTemplatesChanged();

private slots:
    void onCustomTemplatesCountChanged(const QString &file);

private:
    void initLayoutTemplates();
    void initViewTemplates();

    void initLayoutTemplates(const QString &path);
    void initViewTemplates(const QString &path);

    void exposeTranslatedTemplateNames();

    QString uniqueLayoutTemplateName(QString name) const;
    QString uniqueViewTemplateName(QString name) const;

private:
    Latte::Corona *m_corona;

    Data::LayoutsTable m_layoutTemplates;
    Data::GenericTable<Data::Generic> m_viewTemplates;

};

}
}

#endif //TEMPLATESMANAGER_H
