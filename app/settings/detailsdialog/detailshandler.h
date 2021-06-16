/*
    SPDX-FileCopyrightText: 2020 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DETAILSDIALOGHANDLER_H
#define DETAILSDIALOGHANDLER_H

// local
#include "../generic/generichandler.h"
#include "../../data/layoutdata.h"
#include "../../layout/abstractlayout.h"

// Qt
#include <QButtonGroup>
#include <QSortFilterProxyModel>

// KDE
#include <KMessageBox>

namespace Ui {
class DetailsDialog;
}

namespace Latte{
namespace Settings{
namespace Dialog{
class DetailsDialog;
}
}
}

namespace Latte{
namespace Settings{
namespace Model {
class Colors;
class Schemes;
}
}
}


namespace Latte {
namespace Settings {
namespace Handler {

//! Handlers are objects to handle the UI elements that semantically associate with specific
//! ui::tabs or different windows. They are responsible also to handle the user interaction
//! between controllers and views

class DetailsHandler : public Generic
{
    Q_OBJECT
public:
    DetailsHandler(Dialog::DetailsDialog *dialog);
    ~DetailsHandler();

    bool hasChangedData() const override;
    bool inDefaultValues() const override;

    Latte::Data::Layout currentData() const;

public slots:
    void reset() override;
    void resetDefaults() override;
    void save() override;

signals:
    void currentLayoutChanged();

private slots:
    void onCurrentLayoutIndexChanged(int row);
    void onCurrentColorIndexChanged(int row);
    void onCurrentSchemeIndexChanged(int row);

    void clearIcon();
    void clearPattern();
    void selectBackground();
    void selectIcon();
    void selectTextColor();
    void updateWindowTitle();
    void updateCustomSchemeCmb(const int &row);

private:
    void init();
    void reload();

    void setIsShownInMenu(bool inMenu);
    void setHasDisabledBorders(bool disabled);

    void setBackground(const QString &background);
    void setCustomSchemeFile(const QString &file);
    void setTextColor(const QString &textColor);
    void setColor(const QString &color);
    void setIcon(const QString &icon);

    void setPopUpMargin(const int &margin);

    void setBackgroundStyle(const Latte::Layout::BackgroundStyle &style);

    void loadLayout(const Latte::Data::Layout &data);

    KMessageBox::ButtonCode saveChangesConfirmation();

private:
    Dialog::DetailsDialog *m_dialog{nullptr};
    Ui::DetailsDialog *m_ui{nullptr};

    QSortFilterProxyModel *m_layoutsProxyModel{nullptr};

    int m_lastConfirmedLayoutIndex{-1};

    //! current data
    Model::Colors *m_colorsModel{nullptr};
    Model::Schemes *m_schemesModel{nullptr};

    QButtonGroup *m_backButtonsGroup;

    Latte::Data::Layout o_data;
    Latte::Data::Layout c_data;
};

}
}
}

#endif
