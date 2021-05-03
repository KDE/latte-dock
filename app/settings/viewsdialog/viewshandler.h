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

#ifndef VIEWSDIALOGHANDLER_H
#define VIEWSDIALOGHANDLER_H

// local
#include "../generic/generichandler.h"
#include "../../data/layoutdata.h"
#include "../../layout/abstractlayout.h"

// Qt
#include <QAction>
#include <QButtonGroup>
#include <QMenu>
#include <QSortFilterProxyModel>

// KDE
#include <KMessageBox>

namespace Ui {
class ViewsDialog;
}

namespace Latte{
class Corona;
namespace Settings{
namespace Controller{
class Layouts;
class Views;
}
namespace Dialog{
class ViewsDialog;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class ViewsHandler : public Generic
{
    Q_OBJECT
public:
    ViewsHandler(Dialog::ViewsDialog *dialog);
    ~ViewsHandler();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    bool isSelectedLayoutOriginal() const;

    Latte::Data::Layout currentData() const;
    Latte::Data::Layout originalData() const;

    Ui::ViewsDialog *ui() const;
    Latte::Corona *corona() const;
    Settings::Controller::Layouts *layoutsController() const;

public slots:
    void reset() override;
    void resetDefaults() override;
    void save() override;

signals:
    void currentLayoutChanged();

private slots:
    void initViewTemplatesSubMenu();
    void initViewExportSubMenu();
    void removeSelectedViews();
    void updateWindowTitle();

    void exportViewForBackup();
    void exportViewAsTemplate();
    void importView();

    void onCurrentLayoutIndexChanged(int row);

    void newView(const Data::Generic &templateData);

private:
    void init();

    void reload();

    void loadLayout(const Latte::Data::Layout &data);

    KMessageBox::ButtonCode saveChangesConfirmation();
    KMessageBox::ButtonCode removalConfirmation(const int &count);

private:
    Dialog::ViewsDialog *m_dialog{nullptr};
    Ui::ViewsDialog *m_ui{nullptr};
    Settings::Controller::Views *m_viewsController{nullptr};

    QSortFilterProxyModel *m_layoutsProxyModel{nullptr};

    Latte::Data::Layout o_data;

    int m_lastConfirmedLayoutIndex{-1};

    //! Actions
    QAction *m_newViewAction{nullptr};
    QAction *m_duplicateViewAction{nullptr};
    QAction *m_removeViewAction{nullptr};
    QAction *m_exportViewAction{nullptr};
    QAction *m_importViewAction{nullptr};

    //! Menus
    QMenu *m_viewTemplatesSubMenu{nullptr};
    QMenu *m_viewExportSubMenu{nullptr};
};

}
}
}

#endif
