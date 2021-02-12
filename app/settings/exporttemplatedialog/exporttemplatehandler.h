/*
 * Copyright 2021  Michail Vourlakos <mvourlakos@gmail.com>
 *
 * This file is part of Latte-Dock
 *
 * Latte-Dock is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Latte-Dock is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EXPORTTEMPLATEHANDLER_H
#define EXPORTTEMPLATEHANDLER_H

// local
#include "../generic/generichandler.h"
#include "../../data/appletdata.h"

// Qt
#include <QButtonGroup>
#include <QSortFilterProxyModel>

namespace Ui {
class ExportTemplateDialog;
}

namespace Latte{
class View;
namespace Settings{
namespace Dialog{
class ExportTemplateDialog;
}
}
}

namespace Latte{
namespace Settings{
namespace Model {
class Applets;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class ExportTemplateHandler : public Generic
{
    Q_OBJECT
public:
    ExportTemplateHandler(Dialog::ExportTemplateDialog *parentDialog);
    ExportTemplateHandler(Dialog::ExportTemplateDialog *parentDialog, const QString &layoutName, const QString &layoutId);
    ExportTemplateHandler(Dialog::ExportTemplateDialog *parentDialog, Latte::View *view);
    ~ExportTemplateHandler();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    Latte::Data::AppletsTable currentData() const;

public slots:
    void reset() override;
    void resetDefaults() override;
    void save() override;

signals:
    void filepathChanged();

private:
    void init();
    void initDefaults();

    void loadLayoutApplets(const QString &layoutName, const QString &layoutId);
    void loadViewApplets(Latte::View *view);

    void setFilepath(const QString &filepath);

private slots:
    void onFilepathChanged();
    void onSelectAll();
    void onDeselectAll();

    void chooseFileDialog();

private:
    QString c_filepath;
    QString o_filepath;

    Dialog::ExportTemplateDialog *m_parentDialog{nullptr};
    Ui::ExportTemplateDialog *m_ui{nullptr};

    //! current data
    Model::Applets *m_appletsModel{nullptr};
    QSortFilterProxyModel *m_appletsProxyModel{nullptr};
};

}
}
}

#endif
