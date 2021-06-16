/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "schemesmodel.h"

// local
#include "../../data/layoutdata.h"
#include "../../layouts/importer.h"
#include "../../wm/schemecolors.h"

// Qt
#include <QDebug>
#include <QDir>

// KDE
#include <KLocalizedString>

namespace Latte {
namespace Settings {
namespace Model {

Schemes::Schemes(QObject *parent)
    : QAbstractListModel(parent)
{
    initSchemes();
}

Schemes::~Schemes()
{
    qDeleteAll(m_schemes);
}

int Schemes::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_schemes.count();
}

void Schemes::initSchemes()
{
    qDeleteAll(m_schemes);
    m_schemes.clear();

    QString currentSchemePath = WindowSystem::SchemeColors::possibleSchemeFile(Data::Layout::DEFAULTSCHEMEFILE);
    insertSchemeInList(currentSchemePath);

    QStringList standardPaths = Latte::Layouts::Importer::standardPathsFor("color-schemes");

    QStringList registeredSchemes;

    for(auto path : standardPaths) {
        QDir directory(path);
        QStringList tempSchemes = directory.entryList(QStringList() << "*.colors" << "*.COLORS", QDir::Files);

        foreach (QString filename, tempSchemes) {
            if (!registeredSchemes.contains(filename)) {
                QString fullPath = path + "/" + filename;
                insertSchemeInList(fullPath);
                registeredSchemes << filename;
            }
        }
    }
}

void Schemes::insertSchemeInList(QString file)
{
    WindowSystem::SchemeColors *tempScheme = new WindowSystem::SchemeColors(this, file);

    int atPos{0};

    for (int i = 0; i < m_schemes.count(); i++) {
        WindowSystem::SchemeColors *s = m_schemes[i];

        int result = QString::compare(tempScheme->schemeName(), s->schemeName(), Qt::CaseInsensitive);

        if (result < 0) {
            atPos = i;
            break;
        } else {
            atPos = i + 1;
        }

    }

    m_schemes.insert(atPos, tempScheme);
}

int Schemes::row(const QString &id)
{
    if (id.isEmpty() || id == Data::Layout::DEFAULTSCHEMEFILE) {
        return 0;
    }

    for (int i = 0; i < m_schemes.count(); i++) {
        if (m_schemes[i]->schemeFile() == id) {
            return i;
        }
    }

    return -1;
}

QVariant Schemes::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.column() != 0 || index.row() < 0 || index.row() >= m_schemes.count()) {
        return QVariant();
    }

    const WindowSystem::SchemeColors *d = m_schemes[index.row()];

    switch (role) {
    case IDROLE:
        return index.row() == 0 ? Data::Layout::DEFAULTSCHEMEFILE : d->schemeFile();
        break;

    case Qt::DisplayRole:
    case NAMEROLE:
        return index.row() == 0 ? i18n("System Colors") : d->schemeName();
        break;

    case TEXTCOLORROLE:
        return d->textColor();
        break;

    case BACKGROUNDCOLORROLE:
        return d->backgroundColor();
        break;
    }

    return QVariant();
}

}
}
}

