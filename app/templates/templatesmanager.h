/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEMPLATESMANAGER_H
#define TEMPLATESMANAGER_H

// local
#include "../lattecorona.h"
#include "../data/appletdata.h"
#include "../data/layoutdata.h"
#include "../data/layoutstable.h"
#include "../data/genericbasictable.h"

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
    Data::GenericBasicTable viewTemplates();

    //! creates a new layout with layoutName based on specific layout template and returns the new layout path
    QString newLayout(QString layoutName, QString layoutTemplate = i18n(DEFAULTLAYOUTTEMPLATENAME));

    QString proposedTemplateAbsolutePath(QString templateFilename);

    QString viewTemplateFilePath(const QString templateName) const;

    static QString templateName(const QString &filePath);

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
    Data::GenericBasicTable m_viewTemplates;

};

}
}

#endif //TEMPLATESMANAGER_H
