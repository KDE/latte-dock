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

#include "exporttemplatehandler.h"

// local
#include <coretypes.h>
#include "ui_exporttemplatedialog.h"
#include "exporttemplatedialog.h"
#include "appletsmodel.h"
#include "delegates/normalcheckboxdelegate.h"
#include "../settingsdialog/layoutscontroller.h"
#include "../../lattecorona.h"
#include "../../data/appletdata.h"
#include "../../layout/genericlayout.h"
#include "../../layouts/storage.h"
#include "../../templates/templatesmanager.h"
#include "../../view/view.h"

// Qt
#include <QAction>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>

// KDE
#include <KLocalizedString>
#include <KMessageBox>
#include <KIO/OpenFileManagerWindowJob>

// Plasma
#include <Plasma/Containment>

namespace Latte {
namespace Settings {
namespace Handler {

ExportTemplateHandler::ExportTemplateHandler(Dialog::ExportTemplateDialog *dialog)
    : Generic(dialog),
      m_dialog(dialog),
      m_ui(m_dialog->ui()),
      m_appletsModel(new Model::Applets(this))
{
    init();
}

ExportTemplateHandler::ExportTemplateHandler(Dialog::ExportTemplateDialog *dialog, const QString &layoutName, const QString &layoutId)
    : ExportTemplateHandler(dialog)
{
    loadLayoutApplets(layoutName, layoutId);
    o_filepath = dialog->corona()->templatesManager()->proposedTemplateAbsolutePath(layoutName + ".layout.latte");
    setFilepath(o_filepath);
}

ExportTemplateHandler::ExportTemplateHandler(Dialog::ExportTemplateDialog *dialog, Latte::View *view)
    : ExportTemplateHandler(dialog)
{
    QString type = (view->type() == Latte::Types::PanelView ? i18n("Panel") : i18n("Dock"));
    loadViewApplets(view);
    o_filepath = dialog->corona()->templatesManager()->proposedTemplateAbsolutePath(view->layout()->name() + " " + type + ".view.latte");
    setFilepath(o_filepath);
}

ExportTemplateHandler::~ExportTemplateHandler()
{
}

void ExportTemplateHandler::init()
{
    m_ui->appletsTable->horizontalHeader()->setStretchLastSection(true);
    m_ui->appletsTable->horizontalHeader()->setSectionsClickable(false);

    m_ui->appletsTable->verticalHeader()->setVisible(false);

    m_ui->appletsTable->setItemDelegateForColumn(Model::Applets::NAMECOLUMN, new Settings::Applets::Delegate::NormalCheckBox(this));

    //! Data Changed
    connect(this, &ExportTemplateHandler::filepathChanged, this, &ExportTemplateHandler::dataChanged);
    connect(m_appletsModel, &Settings::Model::Applets::appletsDataChanged, this, &ExportTemplateHandler::dataChanged);

    //! Applets Model
    m_appletsProxyModel = new QSortFilterProxyModel(this);
    m_appletsProxyModel->setSourceModel(m_appletsModel);
    m_appletsProxyModel->setSortRole(Model::Applets::SORTINGROLE);
    m_appletsProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    //  m_appletsProxyModel->sort(Model::Applets::NAMECOLUMN, Qt::AscendingOrder);

    m_ui->appletsTable->setModel(m_appletsProxyModel);

    //! Buttons
    connect(m_ui->deselectAllBtn, &QPushButton::clicked, this, &ExportTemplateHandler::onDeselectAll);
    connect(m_ui->selectAllBtn, &QPushButton::clicked, this, &ExportTemplateHandler::onSelectAll);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &ExportTemplateHandler::reset);

    connect(m_ui->chooseBtn, &QPushButton::clicked, this, &ExportTemplateHandler::chooseFileDialog);
    connect(m_dialog->exportButton(), &QPushButton::clicked, this, &ExportTemplateHandler::onExport);

    //! Labels
    connect(this, &ExportTemplateHandler::filepathChanged, this, &ExportTemplateHandler::onFilepathChanged);
}

void ExportTemplateHandler::setFilepath(const QString &filepath)
{
    if (c_filepath == filepath) {
        return;
    }

    c_filepath = filepath;
    emit filepathChanged();
}

void ExportTemplateHandler::loadLayoutApplets(const QString &layoutName, const QString &layoutId)
{
    m_originLayoutFilePath = layoutId;
    Data::AppletsTable c_data = Latte::Layouts::Storage::self()->plugins(layoutId);
    m_appletsModel->setData(c_data);
    m_dialog->setWindowTitle(i18n("Export Layout Template"));
}

void ExportTemplateHandler::loadViewApplets(Latte::View *view)
{
    m_originView = view;
    Data::AppletsTable c_data = Latte::Layouts::Storage::self()->plugins(view->layout(), view->containment()->id());
    m_appletsModel->setData(c_data);
    m_dialog->setWindowTitle(i18n("Export View Template"));
}

void ExportTemplateHandler::chooseFileDialog()
{
    QFileInfo currentFile(c_filepath);
    bool inLayoutState = c_filepath.endsWith("layout.latte");

    QFileDialog *chooseFileDlg = new QFileDialog(m_dialog,
                                                 inLayoutState ? i18n("Choose Layout Template file") : i18n("Choose View Template file"),
                                                 currentFile.absoluteFilePath(),
                                                 inLayoutState ? QStringLiteral(".layout.latte") : QStringLiteral(".view.latte"));

    chooseFileDlg->setLabelText(QFileDialog::Accept, i18nc("choose file","Choose"));
    chooseFileDlg->setFileMode(QFileDialog::AnyFile);
    chooseFileDlg->setAcceptMode(QFileDialog::AcceptSave);
    if (inLayoutState) {
        chooseFileDlg->setDefaultSuffix("layout.latte");
    } else {
        chooseFileDlg->setDefaultSuffix("view.latte");
    }

    QStringList filters;

    if (inLayoutState) {
        filters << QString(i18nc("layout template", "Latte Dock Layout Template file v0.2") + "(*.layout.latte)");
    } else {
        filters << QString(i18nc("view template", "Latte Dock View Template file v0.2") + "(*.view.latte)");
    }

    chooseFileDlg->setNameFilters(filters);

    connect(chooseFileDlg, &QFileDialog::finished, chooseFileDlg, &QFileDialog::deleteLater);
    connect(chooseFileDlg, &QFileDialog::fileSelected, this, [&, inLayoutState](const QString &file) {
        if (inLayoutState) {
            if (!file.endsWith(".layout.latte")) {
                QString selected = file;
                selected = selected.replace(QDir::homePath(), "~");
                showInlineMessage(i18n("<i>%1</i> does not end with <i>.layout.latte</i> extension. Selected file <b>rejected</b>.", selected),
                                  KMessageWidget::Error,
                                  true);
            } else {
                setFilepath(file);
            }
        } else {
            if (!file.endsWith(".view.latte")) {
                QString selected = file;
                selected = selected.replace(QDir::homePath(), "~");
                showInlineMessage(i18n("<i>%1</i> does not end with <i>.view.latte</i> extension. Selected file <b>rejected</b>.", selected),
                                  KMessageWidget::Error,
                                  true);
            } else {
                setFilepath(file);
            }
        }
    });

    chooseFileDlg->open();
    chooseFileDlg->selectFile(currentFile.baseName());
}

void ExportTemplateHandler::onExport()
{
    QString curbasename = QFileInfo(c_filepath).baseName();
    QString curfilename = QFileInfo(c_filepath).fileName();

    //! Confirm Overwrite if that is the case
    if (QFile(c_filepath).exists()) {
        if (!overwriteConfirmation(curfilename)) {
            return;
        }
    }

    //! Proceed with export
    auto showExportTemplateError = [this](const QString &templateName) {
        showInlineMessage(i18nc("settings:template export fail","Template <b>%1</b> export <b>failed</b>...", templateName),
                          KMessageWidget::Error,
                          true);
    };

    bool result{false};

    if (!m_originLayoutFilePath.isEmpty()) {
        result = m_dialog->corona()->templatesManager()->exportTemplate(m_originLayoutFilePath,
                                                                        c_filepath,
                                                                        m_appletsModel->selectedApplets());
    } else if (m_originView){
        result = m_dialog->corona()->templatesManager()->exportTemplate(m_originView,
                                                                        c_filepath,
                                                                        m_appletsModel->selectedApplets());
    }

    if (result) {
        QAction *openUrlAction = new QAction(i18n("Open Location..."), this);
        openUrlAction->setIcon(QIcon::fromTheme("document-open"));
        openUrlAction->setData(c_filepath);
        QList<QAction *> actions;
        actions << openUrlAction;

        connect(openUrlAction, &QAction::triggered, this, [&, openUrlAction]() {
            QString file = openUrlAction->data().toString();

            if (!file.isEmpty()) {
                KIO::highlightInFileManager({file});
            }
        });

        showInlineMessage(i18nc("settings:template export success","Template <b>%1</b> export succeeded...", curbasename),
                          KMessageWidget::Positive,
                          false,
                          actions);

        emit exportSucceeded();
    } else {
        showExportTemplateError(QFileInfo(c_filepath).baseName());
    }
}

void ExportTemplateHandler::onFilepathChanged()
{
    QString filepath = c_filepath;

    filepath = filepath.replace(QDir::homePath(), "~");
    m_ui->fileLbl->setText(filepath);
}

void ExportTemplateHandler::onSelectAll()
{
    m_appletsModel->selectAll();

}

void ExportTemplateHandler::onDeselectAll()
{
    m_appletsModel->deselectAll();
}

bool ExportTemplateHandler::hasChangedData() const
{
    return (c_filepath != o_filepath) || m_appletsModel->hasChangedData();
}

bool ExportTemplateHandler::inDefaultValues() const
{
    return (c_filepath == o_filepath) && m_appletsModel->inDefaultValues();
}

void ExportTemplateHandler::reset()
{
    setFilepath(o_filepath);

    if (m_appletsModel->hasChangedData()) {
        m_appletsModel->reset();
    }
}

void ExportTemplateHandler::resetDefaults()
{
    reset();
}

void ExportTemplateHandler::save()
{
    //do nothing
}

bool ExportTemplateHandler::overwriteConfirmation(const QString &fileName)
{
    return (KMessageBox::warningYesNo(m_dialog,
                                      i18n("The file \"%1\" already exists. Do you wish to overwrite it?", fileName),
                                      i18n("Overwrite File?"),
                                      KStandardGuiItem::overwrite(),
                                      KStandardGuiItem::cancel()) == KMessageBox::Yes);
}

}
}
}
