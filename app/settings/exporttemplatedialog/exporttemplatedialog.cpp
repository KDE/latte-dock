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

#include "exporttemplatedialog.h"

// local
#include "ui_exporttemplatedialog.h"
#include "exporttemplatehandler.h"
#include "../settingsdialog/layoutscontroller.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"
#include "../../settings/universalsettings.h"
#include "../../view/view.h"

namespace Latte {
namespace Settings {
namespace Dialog {

ExportTemplateDialog::ExportTemplateDialog(SettingsDialog *parent, const QString &layoutName, const QString &layoutId)
    : GenericDialog(parent),
      m_ui(new Ui::ExportTemplateDialog)
{
    m_corona = parent->corona();

    init();
    initExportButton(i18n("Export your selected layout as template"));
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ExportTemplateHandler(this, layoutName, layoutId);
    connect(m_handler, &Handler::ExportTemplateHandler::dataChanged, this, &ExportTemplateDialog::onDataChanged);
}

ExportTemplateDialog::ExportTemplateDialog(Latte::View *view)
    : GenericDialog(nullptr),
      m_ui(new Ui::ExportTemplateDialog)
{
    m_corona = qobject_cast<Latte::Corona *>(view->corona());

    init();
    initExportButton(i18n("Export your selected view as template"));
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ExportTemplateHandler(this, view);
}

ExportTemplateDialog::~ExportTemplateDialog()
{
}

Latte::Corona *ExportTemplateDialog::corona() const
{
    return m_corona;
}

Ui::ExportTemplateDialog *ExportTemplateDialog::ui() const
{
    return m_ui;
}

void ExportTemplateDialog::init()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    //! first we need to setup the ui
    m_ui->setupUi(this);
    initButtons();

    //! Update ALL active original layouts before exporting,
    m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();
    m_corona->universalSettings()->syncSettings();
}

void ExportTemplateDialog::initButtons()
{
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
            this, &ExportTemplateDialog::onCancel);
}

void ExportTemplateDialog::initExportButton(const QString &tooltip)
{
    m_exportButton = new QPushButton(this);
    m_exportButton->setText(i18nc("export template", "Export"));
    m_exportButton->setIcon(QIcon::fromTheme("document-export"));
    m_exportButton->setToolTip(tooltip);

    m_ui->buttonBox->addButton(m_exportButton, QDialogButtonBox::AcceptRole);
}

QPushButton *ExportTemplateDialog::exportButton() const
{
    return m_exportButton;
}

void ExportTemplateDialog::onDataChanged()
{
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(!m_handler->inDefaultValues());
}

void ExportTemplateDialog::accept()
{
    qDebug() << Q_FUNC_INFO;
    //close();
}

void ExportTemplateDialog::onCancel()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void ExportTemplateDialog::onReset()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

}
}
}
