/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LAYOUTSSTORAGE_H
#define LAYOUTSSTORAGE_H

// local
#include "../data/appletdata.h"
#include "../data/errordata.h"
#include "../data/genericdata.h"
#include "../data/generictable.h"
#include "../data/viewstable.h"

// Qt
#include <QTemporaryDir>

// KDE
#include <KConfigGroup>

// Plasma
#include <Plasma/Applet>
#include <Plasma/Containment>

namespace Latte {
class Corona;
namespace Layout {
class GenericLayout;
}
}

namespace Latte {
namespace Layouts {

struct SubContaimentIdentityData
{
    QString cfgGroup;
    QString cfgProperty;
};

class Storage
{

public:
    static Storage *self();
    ~Storage();

    static const int IDNULL;
    static const int IDBASE;

    bool isWritable(const Layout::GenericLayout *layout) const;
    bool isLatteContainment(const Plasma::Containment *containment) const;
    bool isLatteContainment(const KConfigGroup &group) const;
    bool isSubContainment(const Plasma::Corona *corona, const Plasma::Applet *applet) const;

    bool hasContainment(const Layout::GenericLayout *layout, const int &id);
    bool containsView(const QString &filepath, const int &viewId);

    bool isClonedView(const Plasma::Containment *containment) const;
    bool isClonedView(const KConfigGroup &containmentGroup) const;
    void removeAllClonedViews(const QString &filepath);

    int subContainmentId(const KConfigGroup &appletGroup) const;

    Plasma::Containment *subContainmentOf(const Plasma::Corona *corona, const Plasma::Applet *applet);

    void lock(const Layout::GenericLayout *layout); //! make it only read-only
    void unlock(const Layout::GenericLayout *layout); //! make it writable which it should be the default

    void importToCorona(const Layout::GenericLayout *layout);
    void syncToLayoutFile(const Layout::GenericLayout *layout, bool removeLayoutId);

    Data::View newView(const Layout::GenericLayout *destination, const Data::View &nextViewData);
    void removeView(const QString &filepath, const Data::View &viewData);
    void updateView(const Layout::GenericLayout *layout, const Data::View &viewData);
    void updateView(KConfigGroup viewGroup, const Data::View &viewData);
    QString storedView(const Layout::GenericLayout *layout, const int &containmentId); //returns temp filepath containing all view data

    void moveToLayoutFile(const QString &layoutName);
    QStringList storedLayoutsInMultipleFile();

    void removeContainment(const QString &filepath, const QString &containmentId);

    bool exportTemplate(const QString &originFile, const QString &destinationFile, const Data::AppletsTable &approvedApplets);
    bool exportTemplate(const Layout::GenericLayout *layout, Plasma::Containment *containment, const QString &destinationFile, const Data::AppletsTable &approvedApplets);

    int expectedViewScreenId(const Latte::Corona *corona, const Data::View &view) const;
    int expectedViewScreenId(const Latte::Corona *corona, const KConfigGroup &containmentGroup) const;
    int expectedViewScreenId(const Layout::GenericLayout *layout, const Plasma::Containment *lattecontainment) const;

    /// STATIC
    //! Check if an applet config group is valid or belongs to removed applet
    static bool appletGroupIsValid(const KConfigGroup &appletGroup);
    static bool isValid(const int &id);

    //! AppletsData Information
    Data::Applet metadata(const QString &pluginId);
    Data::AppletsTable plugins(const Layout::GenericLayout *layout, const int containmentid = IDNULL);
    Data::AppletsTable plugins(const QString &layoutfile, const int containmentid = IDNULL);

    Data::GenericTable<Data::Generic> subcontainments(const KConfigGroup &containmentGroup);
    Data::GenericTable<Data::Generic> subcontainments(const Layout::GenericLayout *layout, const Plasma::Containment *lattecontainment) const;
    Data::View view(const KConfigGroup &containmentGroup);
    Data::View view(const Layout::GenericLayout *layout, const Plasma::Containment *lattecontainment);
    Data::ViewsTable views(const QString &file);
    Data::ViewsTable views(const Layout::GenericLayout *layout);

    //! errors/warning;
    Data::ErrorsList errors(const Layout::GenericLayout *layout);
    Data::WarningsList warnings(const Layout::GenericLayout *layout);

private:
    Storage();

    void clearExportedLayoutSettings(KConfigGroup &layoutSettingsGroup);
    void importContainments(const QString &originFile, const QString &destinationFile);
    void syncContainmentConfig(Plasma::Containment *containment);

    bool isSubContainment(const KConfigGroup &appletGroup) const;
    int subIdentityIndex(const KConfigGroup &appletGroup) const;

    //! STORAGE !////
    QString availableId(QStringList all, QStringList assigned, int base);
    //! provides a new file path based the provided file. The new file
    //! has updated ids for containments and applets based on the corona
    //! loaded ones
    QString newUniqueIdsFile(QString originFile, const Layout::GenericLayout *destinationLayout);
    //! imports a layout file and returns the containments for the docks
    QList<Plasma::Containment *> importLayoutFile(const Layout::GenericLayout *layout, QString file);

    QStringList containmentsIds(const QString &filepath);
    QStringList appletsIds(const QString &filepath);

    //! errors checkers
    bool hasDifferentAppletsWithSameId(const Layout::GenericLayout *layout, Data::Error &error);
    bool hasOrphanedParentAppletOfSubContainment(const Layout::GenericLayout *layout, Data::Error &error);
    //! warnings checkers
    bool hasAppletsAndContainmentsWithSameId(const Layout::GenericLayout *layout, Data::Warning &warning);
    bool hasOrphanedSubContainments(const Layout::GenericLayout *layout, Data::Warning &warning);
private:
    QTemporaryDir m_storageTmpDir;

    Data::GenericTable<Data::Generic> s_knownErrors;

    QList<SubContaimentIdentityData> m_subIdentities;
};

}
}

#endif
