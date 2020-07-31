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

#include "indicatoruimanager.h"

// local
#include "primaryconfigview.h"
#include "../view.h"
#include "../../lattecorona.h"
#include "../../indicator/factory.h"

// Qt
#include <QFileDialog>
#include <QTimer>

// KDE
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KDeclarative/QmlObjectSharedEngine>

namespace Latte {
namespace ViewPart {
namespace Config {

IndicatorUiManager::IndicatorUiManager(ViewPart::PrimaryConfigView *parent)
    : QObject(parent),
      m_primary(parent)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<Latte::ViewPart::Config::IndicatorUiManager>();
#else
    qmlRegisterAnonymousType<Latte::ViewPart::Config::IndicatorUiManager>("latte-dock", 1);
#endif
}

IndicatorUiManager::~IndicatorUiManager()
{
}

bool IndicatorUiManager::contains(const QString &type)
{
    return (index(type) >= 0);
}

int IndicatorUiManager::index(const QString &type)
{
    for (int i=0; i<m_uidata.count(); ++i) {
        if (m_uidata[i].type == type) {
            return i;
        }
    }

    return -1;
}


void IndicatorUiManager::setParentItem(QQuickItem *parentItem)
{
    m_parentItem = parentItem;
}

void IndicatorUiManager::hideAllUi()
{
    for (int i=0; i<m_uidata.count(); ++i) {
        if (m_uidata[i].ui) {
            //! config ui has already been created and can be provided again
            QQuickItem *qmlItem = qobject_cast<QQuickItem*>(m_uidata[i].ui->rootObject());
            if (qmlItem) {
                qmlItem->setVisible(false);
            }
        }
    }
}

void IndicatorUiManager::ui(const QString &type, Latte::View *view)
{
    if (!m_parentItem) {
        return;
    }

    hideAllUi();

    int typeIndex = index(type);

    if (typeIndex > -1 && m_uidata[typeIndex].ui) {
        m_uidata[typeIndex].ui->rootContext()->setContextProperty(QStringLiteral("indicator"), view->indicator());
        m_uidata[typeIndex].view = view;

        //! config ui has already been created and can be provided again
        QQuickItem *qmlItem = qobject_cast<QQuickItem*>(m_uidata[typeIndex].ui->rootObject());
        if (qmlItem) {
            qmlItem->setVisible(true);
        }
        return;
    }

    //! type needs to be created again
    KPluginMetaData metadata = m_primary->corona()->indicatorFactory()->metadata(type);;

    if (metadata.isValid()) {
        QString uiPath = metadata.value("X-Latte-ConfigUi");

        if (!uiPath.isEmpty()) {
            IndicatorUiData uidata;

            uidata.ui = new KDeclarative::QmlObjectSharedEngine(this);
            uidata.pluginPath = metadata.fileName().remove("metadata.desktop");
            uidata.type = type;
            uidata.view = view;

            uidata.ui->setTranslationDomain(QLatin1String("latte_indicator_") + metadata.pluginId());
            uidata.ui->setInitializationDelayed(true);
            uiPath = uidata.pluginPath + "package/" + uiPath;
            uidata.ui->setSource(QUrl::fromLocalFile(uiPath));
            uidata.ui->rootContext()->setContextProperty(QStringLiteral("dialog"), m_parentItem);
            uidata.ui->rootContext()->setContextProperty(QStringLiteral("indicator"), view->indicator());
            uidata.ui->completeInitialization();

            QQuickItem *qmlItem = qobject_cast<QQuickItem*>(uidata.ui->rootObject());
            if (qmlItem) {
                qmlItem->setParentItem(m_parentItem);
            }

            m_uidata << uidata;
        }
    }
}

void IndicatorUiManager::addIndicator()
{
    QFileDialog *fileDialog = new QFileDialog(nullptr
                                              , i18nc("add indicator", "Add Indicator")
                                              , QDir::homePath()
                                              , QStringLiteral("indicator.latte"));

    fileDialog->setFileMode(QFileDialog::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog->setDefaultSuffix("indicator.latte");

    QStringList filters;
    filters << QString(i18nc("add indicator file", "Latte Indicator") + "(*.indicator.latte)");
    fileDialog->setNameFilters(filters);

    connect(fileDialog, &QFileDialog::finished, fileDialog, &QFileDialog::deleteLater);

    connect(fileDialog, &QFileDialog::fileSelected, this, [&](const QString & file) {
        qDebug() << "Trying to import indicator file ::: " << file;
        m_primary->corona()->indicatorFactory()->importIndicatorFile(file);
    });

    fileDialog->open();
}

void IndicatorUiManager::downloadIndicator()
{
    //! call asynchronously in order to not crash when view settings window
    //! loses focus and it closes
    QTimer::singleShot(0, [this]() {
        m_primary->corona()->indicatorFactory()->downloadIndicator();
    });
}

void IndicatorUiManager::removeIndicator(QString pluginId)
{    //! call asynchronously in order to not crash when view settings window
    //! loses focus and it closes
    QTimer::singleShot(0, [this, pluginId]() {
        m_primary->corona()->indicatorFactory()->removeIndicator(pluginId);
    });
}


}
}
}
