/*
 *   Copyright (C) 2016 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Self
#include "sortedactivitiesmodel.h"

// C++
#include <functional>

// Qt
#include <QColor>
#include <QFileInfo>
#include <QImage>
#include <QObject>
#include <QTimer>
#include <QtMath>

// KDE
#include <KSharedConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KActivities/Consumer>

#include <Plasma>

#define PLASMACONFIG "plasma-org.kde.plasma.desktop-appletsrc"

namespace {

class BackgroundCache: public QObject
{
public:
    BackgroundCache()
        : initialized(false)
        , plasmaConfig(KSharedConfig::openConfig(PLASMACONFIG)) {
        using namespace std::placeholders;

        const auto configFile = QStandardPaths::writableLocation(
                                    QStandardPaths::GenericConfigLocation) +
                                QLatin1Char('/') + PLASMACONFIG;

        KDirWatch::self()->addFile(configFile);

        QObject::connect(KDirWatch::self(), &KDirWatch::dirty,
                         this, &BackgroundCache::settingsFileChanged,
                         Qt::QueuedConnection);
        QObject::connect(KDirWatch::self(), &KDirWatch::created,
                         this, &BackgroundCache::settingsFileChanged,
                         Qt::QueuedConnection);

    }

    void settingsFileChanged(const QString &file) {
        if (!file.endsWith(PLASMACONFIG)) {
            return;
        }

        if (initialized) {
            plasmaConfig->reparseConfiguration();
            reload();
        }
    }

    void subscribe(SortedActivitiesModel *model) {
        if (!initialized) {
            reload();
        }

        models << model;
    }

    void unsubscribe(SortedActivitiesModel *model) {
        models.removeAll(model);

        if (models.isEmpty()) {
            initialized = false;
            forActivity.clear();
        }
    }

    QString backgroundFromConfig(const KConfigGroup &config) const {
        auto wallpaperPlugin = config.readEntry("wallpaperplugin");
        auto wallpaperConfig = config.group("Wallpaper").group(wallpaperPlugin).group("General");

        if (wallpaperConfig.hasKey("Image")) {
            // Trying for the wallpaper
            auto wallpaper = wallpaperConfig.readEntry("Image", QString());

            if (!wallpaper.isEmpty()) {
                return wallpaper;
            }
        }

        if (wallpaperConfig.hasKey("Color")) {
            auto backgroundColor = wallpaperConfig.readEntry("Color", QColor(0, 0, 0));
            return backgroundColor.name();
        }

        return QString();
    }

    void reload() {
        auto newForActivity = forActivity;
        QHash<QString, int> lastScreenForActivity;

        // contains activities for which the wallpaper
        // has updated
        QStringList changedActivities;

        // Contains activities not covered by any containment
        QStringList ghostActivities = forActivity.keys();

        // Traversing through all containments in search for
        // containments that define activities in plasma
        for (const auto &containmentId : plasmaConfigContainments().groupList()) {
            const auto containment = plasmaConfigContainments().group(containmentId);
            const auto lastScreen  = containment.readEntry("lastScreen", 0);
            const auto activity    = containment.readEntry("activityId", QString());

            // Ignore the containment if the activity is not defined
            if (activity.isEmpty()) continue;

            // If we have already found the same activity from another
            // containment, we are using the new one only if
            // the previous one was a color and not a proper wallpaper,
            // or if the screen ID is closer to zero
            const bool processed = !ghostActivities.contains(activity) &&
                                   newForActivity.contains(activity) &&
                                   (lastScreenForActivity[activity] < lastScreen);

            // qDebug() << "GREPME Searching containment " << containmentId
            //          << "for the wallpaper of the " << activity << " activity - "
            //          << "currently, we think that the wallpaper is " << processed << (processed ? newForActivity[activity] : QString())
            //          << "last screen is" << lastScreen
            //          ;

            if (processed &&
                newForActivity[activity][0] != '#') continue;

            // Marking the current activity as processed
            ghostActivities.removeAll(activity);

            const auto returnedBackground = backgroundFromConfig(containment);

            QString background = returnedBackground;

            if (background.startsWith("file://")) {
                background = returnedBackground.mid(7);
            }

            // qDebug() << "        GREPME Found wallpaper: " << background;

            if (background.isEmpty()) continue;

            // If we got this far and we already had a new wallpaper for
            // this activity, it means we now have a better one
            bool foundBetterWallpaper = changedActivities.contains(activity);

            if (foundBetterWallpaper || newForActivity[activity] != background) {
                if (!foundBetterWallpaper) {
                    changedActivities << activity;
                }

                // qDebug() << "        GREPME Setting: " << activity << " = " << background << "," << lastScreen;

                newForActivity[activity] = background;
                lastScreenForActivity[activity] = lastScreen;
            }
        }

        initialized = true;

        // Removing the activities from the list if we haven't found them
        // while traversing through the containments
        for (const auto &activity : ghostActivities) {
            newForActivity.remove(activity);
        }

        // If we have detected the changes, lets notify everyone
        if (!changedActivities.isEmpty()) {
            forActivity = newForActivity;

            for (auto model : models) {
                model->onBackgroundsUpdated(changedActivities);
            }
        }
    }

    KConfigGroup plasmaConfigContainments() {
        return plasmaConfig->group("Containments");
    }

    QHash<QString, QString> forActivity;
    QList<SortedActivitiesModel *> models;

    bool initialized;
    KSharedConfig::Ptr plasmaConfig;
};

static BackgroundCache &backgrounds()
{
    // If you convert this to a shared pointer,
    // fix the connections to KDirWatcher
    static BackgroundCache cache;
    return cache;
}

}

SortedActivitiesModel::SortedActivitiesModel(const QVector<KActivities::Info::State> &states, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_activitiesModel(new KActivities::ActivitiesModel(states, this))
    , m_activities(new KActivities::Consumer(this))
{
    setSourceModel(m_activitiesModel);

    setDynamicSortFilter(true);
    setSortRole(KActivities::ActivitiesModel::ActivityId);
    sort(0, Qt::DescendingOrder);

    backgrounds().subscribe(this);
}

SortedActivitiesModel::~SortedActivitiesModel()
{
    backgrounds().unsubscribe(this);
}

bool SortedActivitiesModel::inhibitUpdates() const
{
    return m_inhibitUpdates;
}

void SortedActivitiesModel::setInhibitUpdates(bool inhibitUpdates)
{
    if (m_inhibitUpdates != inhibitUpdates) {
        m_inhibitUpdates = inhibitUpdates;
        emit inhibitUpdatesChanged(m_inhibitUpdates);

        setDynamicSortFilter(!inhibitUpdates);
    }
}

/*QHash<int, QByteArray> SortedActivitiesModel::roleNames() const
{
    if (!sourceModel()) return QHash<int, QByteArray>();

    auto roleNames = sourceModel()->roleNames();

    roleNames[LastTimeUsed]       = "lastTimeUsed";
    roleNames[LastTimeUsedString] = "lastTimeUsedString";
    roleNames[WindowCount]        = "windowCount";
    roleNames[HasWindows]         = "hasWindows";

    return roleNames;
}*/

QVariant SortedActivitiesModel::data(const QModelIndex &index, int role) const
{
    if (role == KActivities::ActivitiesModel::ActivityBackground) {
        const auto activity = activityIdForIndex(index);

        return backgrounds().forActivity[activity];

    } else {
        return QSortFilterProxyModel::data(index, role);
    }
}

QString SortedActivitiesModel::activityIdForIndex(const QModelIndex &index) const
{
    return data(index, KActivities::ActivitiesModel::ActivityId).toString();
}

QString SortedActivitiesModel::activityIdForRow(int row) const
{
    return activityIdForIndex(index(row, 0));
}

int SortedActivitiesModel::rowForActivityId(const QString &activity) const
{
    int position = -1;

    for (int row = 0; row < rowCount(); ++row) {
        if (activity == activityIdForRow(row)) {
            position = row;
        }
    }

    return position;
}

QString SortedActivitiesModel::relativeActivity(int relative) const
{
    const auto currentActivity = m_activities->currentActivity();

    if (!sourceModel()) return QString();

    const auto currentRowCount = sourceModel()->rowCount();

    //x % 0 is undefined in c++
    if (currentRowCount == 0) {
        return QString();
    }

    int currentActivityRow = 0;

    for (; currentActivityRow < currentRowCount; currentActivityRow++) {
        if (activityIdForRow(currentActivityRow) == currentActivity) break;
    }

    currentActivityRow = currentActivityRow + relative;

    //wrap to within bounds for both positive and negative currentActivityRows
    currentActivityRow = (currentRowCount + (currentActivityRow % currentRowCount)) % currentRowCount;

    return activityIdForRow(currentActivityRow);
}

void SortedActivitiesModel::onCurrentActivityChanged(const QString &currentActivity)
{
    if (m_previousActivity == currentActivity) return;

    const int previousActivityRow = rowForActivityId(m_previousActivity);
//    emit rowChanged(previousActivityRow);

    m_previousActivity = currentActivity;

    const int currentActivityRow = rowForActivityId(m_previousActivity);
    //  emit rowChanged(currentActivityRow);
}

void SortedActivitiesModel::onBackgroundsUpdated(const QStringList &activities)
{
    for (const auto &activity : activities) {
        const int row = rowForActivityId(activity);
        emit rowChanged(row, { KActivities::ActivitiesModel::ActivityBackground });
    }
}


void SortedActivitiesModel::rowChanged(int row, const QVector<int> &roles)
{
    if (row == -1) return;

    emit dataChanged(index(row, 0), index(row, 0), roles);
}

float SortedActivitiesModel::luminasFromFile(QString imageFile, int edge)
{
    QImage image(imageFile);

    Plasma::Types::Location location = static_cast<Plasma::Types::Location>(edge);

    if (m_luminasCache.keys().contains(imageFile)) {
        if (m_luminasCache[imageFile].keys().contains(location)) {
            return m_luminasCache[imageFile].value(location);
        }
    }

    if (image.format() != QImage::Format_Invalid) {
        int maskHeight = (0.08 * image.height());
        int maskWidth = (0.05 * image.width());

        float areaLumin = -1000;

        int firstRow = 0;
        int firstColumn = 0;
        int endRow = 0;
        int endColumn = 0;

        if (location == Plasma::Types::TopEdge) {
            firstRow = 0;
            endRow = maskHeight;
            firstColumn = 0;
            endColumn = image.width() - 1;
        } else if (location == Plasma::Types::BottomEdge) {
            firstRow = image.height() - maskHeight - 1;
            endRow = image.height() - 1;
            firstColumn = 0;
            endColumn = image.width() - 1;
        } else if (location == Plasma::Types::LeftEdge) {
            firstRow = 0;
            endRow = image.height() - 1;
            firstColumn = 0;
            endColumn = maskWidth;
        } else if (location == Plasma::Types::RightEdge) {
            firstRow = 0;
            endRow = image.height() - 1;
            firstColumn = image.width() - 1 - maskWidth;
            endColumn = image.width() - 1;
        }

        for (int row = firstRow; row < endRow; ++row) {
            QRgb *line = (QRgb *)image.scanLine(row);

            for (int col = firstColumn; col < endColumn ; ++col) {
                // formula for luminance according to:
                // https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef
                QRgb pixelData = line[col];
                float r = (float)(qRed(pixelData)) / 255;
                float g = (float)(qGreen(pixelData)) / 255;
                float b = (float)(qBlue(pixelData)) / 255;

                float rS = (r <= 0.03928 ? r / 12.92 : qPow(((r + 0.055) / 1.055), 2.4));
                float gS = (g <= 0.03928 ? g / 12.92 : qPow(((g + 0.055) / 1.055), 2.4));
                float bS = (b <= 0.03928 ? b / 12.92 : qPow(((b + 0.055) / 1.055), 2.4));

                float pixelLuminosity = 0.2126 * rS + 0.7152 * gS + 0.0722 * bS;

                areaLumin = (areaLumin == -1000) ? pixelLuminosity : (areaLumin + pixelLuminosity);
            }
        }

        float areaSize = (endRow - firstRow) * (endColumn - firstColumn);
        areaLumin = areaLumin / areaSize;

        if (!m_luminasCache.keys().contains(imageFile)) {
            m_luminasCache[imageFile] = EdgesHash();
        }

        m_luminasCache[imageFile].insert(location, areaLumin);

        return areaLumin;
    }

    //! didnt find anything
    return -1000;
}
