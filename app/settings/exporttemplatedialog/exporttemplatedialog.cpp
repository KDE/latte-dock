/*
    SPDX-FileCopyrightText: 2021 Michail Vourlakos <mvourlakos@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

ExportTemplateDialog::ExportTemplateDialog(QDialog *parent)
    : GenericDialog(parent),
      m_ui(new Ui::ExportTemplateDialog)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
}

ExportTemplateDialog::ExportTemplateDialog(SettingsDialog *parent, const Data::Layout &layout)
    : ExportTemplateDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_corona = parent->corona();

    init();
    initExportButton(i18n("Export your selected layout as template"));
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ExportTemplateHandler(this, layout);
    initSignals();
}

ExportTemplateDialog::ExportTemplateDialog(ViewsDialog *parent, const Data::View &view)
    : ExportTemplateDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_corona = parent->corona();

    init();
    initExportButton(i18n("Export your selected dock or panel as template"));
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ExportTemplateHandler(this, view);
    initSignals();
}

ExportTemplateDialog::ExportTemplateDialog(Latte::View *view)
    : GenericDialog(nullptr),
      m_ui(new Ui::ExportTemplateDialog)/*this is necessary, in order to create the ui*/
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_corona = qobject_cast<Latte::Corona *>(view->corona());

    init();
    initExportButton(i18n("Export your selected view as template"));
    //! we must create handlers after creating/adjusting the ui
    m_handler = new Handler::ExportTemplateHandler(this, view);
    initSignals();
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

void ExportTemplateDialog::initSignals()
{
    connect(m_handler, &Handler::ExportTemplateHandler::dataChanged, this, &ExportTemplateDialog::onDataChanged);
    connect(m_handler, &Handler::ExportTemplateHandler::exportSucceeded, this, &ExportTemplateDialog::onExportSucceeded);
}

QPushButton *ExportTemplateDialog::exportButton() const
{
    return m_exportButton;
}

void ExportTemplateDialog::onDataChanged()
{
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(!m_handler->inDefaultValues());
}

void ExportTemplateDialog::onExportSucceeded()
{
    m_exportButton->setText(i18nc("export template", "Export Again"));
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
