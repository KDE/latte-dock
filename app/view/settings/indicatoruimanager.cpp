/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "indicatoruimanager.h"

// local
#include "primaryconfigview.h"
#include "../view.h"
#include "../indicator/indicator.h"
#include "../../lattecorona.h"
#include "../../indicator/factory.h"

// Qt
#include <QFileDialog>
#include <QFileInfo>
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
    qmlRegisterAnonymousType<Latte::ViewPart::Config::IndicatorUiManager>("latte-dock", 1);
}

IndicatorUiManager::~IndicatorUiManager()
{
    m_uidata.clear();
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

void IndicatorUiManager::showNextIndicator()
{
    if (!m_parentItem) {
        return;
    }

    if (auto *metaObject = m_parentItem->metaObject()) {
        int methodIndex = metaObject->indexOfMethod("showNextIndicator()");

        if (methodIndex == -1) {
            qDebug() << "indicator parent page function showNextIndicator() was not found...";
            return;
        }

        QMetaMethod method = metaObject->method(methodIndex);
        method.invoke(m_parentItem);
    }
}

void IndicatorUiManager::ui(const QString &type, Latte::View *view)
{
    if (!m_parentItem) {
        return;
    }

    int typeIndex = index(type);

    if (typeIndex > -1 && m_uidata[typeIndex].ui) {
        m_uidata[typeIndex].ui->rootContext()->setContextProperty(QStringLiteral("indicator"), view->indicator());
        m_uidata[typeIndex].view = view;

        //! config ui has already been created and can be provided again
        QQuickItem *qmlItem = qobject_cast<QQuickItem*>(m_uidata[typeIndex].ui->rootObject());
        if (qmlItem) {
            qmlItem->setParentItem(m_parentItem);
            showNextIndicator();
        }
        return;
    }

    //! type needs to be created again
    KPluginMetaData metadata = m_primary->corona()->indicatorFactory()->metadata(type);

    if (metadata.isValid()) {
        QString uiPath = metadata.value("X-Latte-ConfigUi");

        if (!uiPath.isEmpty()) {
            IndicatorUiData uidata;

            uidata.ui = new KDeclarative::QmlObjectSharedEngine(this);
            uidata.pluginPath = QFileInfo(metadata.fileName()).absolutePath();
            uidata.type = type;
            uidata.view = view;

            uidata.ui->setTranslationDomain(QLatin1String("latte_indicator_") + metadata.pluginId());
            uidata.ui->setInitializationDelayed(true);
            uiPath = uidata.pluginPath + "/package/" + uiPath;
            uidata.ui->setSource(QUrl::fromLocalFile(uiPath));
            uidata.ui->rootContext()->setContextProperty(QStringLiteral("dialog"), m_parentItem);
            uidata.ui->rootContext()->setContextProperty(QStringLiteral("indicator"), view->indicator());
            uidata.ui->completeInitialization();

            int newTypeIndex = view->indicator()->index(type);
            int newPos = -1;

            for (int i=0; i<m_uidata.count(); ++i) {
                int oldTypeIndex = view->indicator()->index(m_uidata[i].type);

                if (oldTypeIndex > newTypeIndex) {
                    newPos = i;
                    break;
                }
            }

            if (newPos == -1) {
                m_uidata << uidata;
            } else {
                m_uidata.insert(newPos, uidata);
            }

            QQuickItem *qmlItem = qobject_cast<QQuickItem*>(uidata.ui->rootObject());
            if (qmlItem) {
                qmlItem->setParentItem(m_parentItem);
                showNextIndicator();
            }
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
