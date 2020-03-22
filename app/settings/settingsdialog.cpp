/*
 * Copyright 2017  Smith AR <audoban@openmailbox.org>
 *                 Michail Vourlakos <mvourlakos@gmail.com>
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

#include "settingsdialog.h"

// local
#include "universalsettings.h"
#include "ui_settingsdialog.h"
#include "../lattecorona.h"
#include "../screenpool.h"
#include "../layout/centrallayout.h"
#include "../layouts/importer.h"
#include "../layouts/manager.h"
#include "../layouts/synchronizer.h"
#include "../liblatte2/types.h"
#include "../plasma/extended/theme.h"
#include "data/layoutdata.h"
#include "tools/settingstools.h"

// Qt
#include <QButtonGroup>
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>

// KDE
#include <KIO/OpenFileManagerWindowJob>
#include <KLocalizedString>
#include <KWindowSystem>


#define TWINENABLED "Enabled"
#define TWINVISIBLE "Visible"
#define TWINCHECKED "Checked"

namespace Latte {

const int SettingsDialog::INFORMATIONINTERVAL;
const int SettingsDialog::INFORMATIONWITHACTIONINTERVAL;
const int SettingsDialog::WARNINGINTERVAL;
const int SettingsDialog::ERRORINTERVAL;

SettingsDialog::SettingsDialog(QWidget *parent, Latte::Corona *corona)
    : QDialog(parent),
      m_ui(new Ui::SettingsDialog),
      m_corona(corona)
{
    setAcceptDrops(true);
    m_ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    resize(m_corona->universalSettings()->layoutsWindowSize());

    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked
            , this, &SettingsDialog::apply);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked
            , this, &SettingsDialog::reset);
    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked
            , this, &SettingsDialog::restoreDefaults);

    //! Global Menu
    initGlobalMenu();

    m_tabLayoutsHandler = new Settings::Handler::TabLayouts(this);
    m_preferencesHandler = new Settings::Handler::TabPreferences(this);

    m_ui->messageWidget->setVisible(false);

    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));

    //! SIGNALS
    connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsDialog::updateApplyButtonsState);

    connect(m_tabLayoutsHandler, &Settings::Handler::TabLayouts::dataChanged, this, &SettingsDialog::updateApplyButtonsState);

    connect(m_preferencesHandler, &Settings::Handler::TabPreferences::dataChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(m_preferencesHandler, &Settings::Handler::TabPreferences::borderlessMaximizedChanged,  this, [&]() {
        bool noBordersForMaximized = m_ui->noBordersForMaximizedChkBox->isChecked();

        if (noBordersForMaximized) {
            m_ui->layoutsView->setColumnHidden(Settings::Model::Layouts::BORDERSCOLUMN, false);
        } else {
            m_ui->layoutsView->setColumnHidden(Settings::Model::Layouts::BORDERSCOLUMN, true);
        }
    });

    //! timers
    m_activitiesTimer.setSingleShot(true);
    m_activitiesTimer.setInterval(750);
    connect(&m_activitiesTimer, &QTimer::timeout, this, &SettingsDialog::updateWindowActivities);
    m_activitiesTimer.start();

    m_hideInlineMessageTimer.setSingleShot(true);
    m_hideInlineMessageTimer.setInterval(2000);
    connect(&m_hideInlineMessageTimer, &QTimer::timeout, this, [&]() {
        m_ui->messageWidget->animatedHide();
    });

    connect(m_ui->messageWidget, &KMessageWidget::hideAnimationFinished, this, [&]() {
        clearCurrentMessageActions();
    });

    updateApplyButtonsState();
}

SettingsDialog::~SettingsDialog()
{
    qDebug() << Q_FUNC_INFO;

    m_corona->universalSettings()->setLayoutsWindowSize(size());
}

void SettingsDialog::initGlobalMenu()
{
    m_globalMenuBar = new QMenuBar(this);

    layout()->setMenuBar(m_globalMenuBar);

    initFileMenu();
    initHelpMenu();
}

void SettingsDialog::initFileMenu()
{
    if (!m_fileMenu) {
        m_fileMenu = new QMenu(i18n("File"), m_globalMenuBar);
        m_globalMenuBar->addMenu(m_fileMenu);
    }

    m_importFullAction = m_fileMenu->addAction(i18n("Import Configuration..."));
    m_importFullAction->setIcon(QIcon::fromTheme("document-import"));
    m_importFullAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_I));
    m_importFullAction->setToolTip(i18n("Import your full configuration from previous backup"));
    connect(m_importFullAction, &QAction::triggered, this, &SettingsDialog::on_import_fullconfiguration);

    m_exportFullAction = m_fileMenu->addAction(i18n("Export Configuration..."));
    m_exportFullAction->setIcon(QIcon::fromTheme("document-export"));
    m_exportFullAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_E));
    m_exportFullAction->setToolTip(i18n("Export your full configuration to create backup"));
    connect(m_exportFullAction, &QAction::triggered, this, &SettingsDialog::on_export_fullconfiguration);

    m_fileMenu->addSeparator();

    QAction *screensAction = m_fileMenu->addAction(i18n("Sc&reens..."));
    screensAction->setIcon(QIcon::fromTheme("document-properties"));
    //screensAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));

    QAction *quitAction = m_fileMenu->addAction(i18n("&Quit Latte"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));


    //! triggers
    connect(quitAction, &QAction::triggered, this, [&]() {
        close();
        m_corona->quitApplication();
    });

}

void SettingsDialog::initHelpMenu()
{
    if (!m_helpMenu) {
        m_helpMenu = new KHelpMenu(m_globalMenuBar);
        m_globalMenuBar->addMenu(m_helpMenu->menu());
    }

    //! hide help menu actions that are not used
    m_helpMenu->action(KHelpMenu::menuHelpContents)->setVisible(false);
    m_helpMenu->action(KHelpMenu::menuWhatsThis)->setVisible(false);
}

Latte::Corona *SettingsDialog::corona() const
{
    return m_corona;
}

Ui::SettingsDialog *SettingsDialog::ui() const
{
    return m_ui;
}

QMenuBar *SettingsDialog::appMenuBar() const
{
    return m_globalMenuBar;
}

Types::LatteConfigPage SettingsDialog::currentPage()
{
    Types::LatteConfigPage cPage= static_cast<Types::LatteConfigPage>(m_ui->tabWidget->currentIndex());

    return cPage;
}

void SettingsDialog::toggleCurrentPage()
{
    if (m_ui->tabWidget->currentIndex() == 0) {
        m_ui->tabWidget->setCurrentIndex(1);
    } else {
        m_ui->tabWidget->setCurrentIndex(0);
    }
}

void SettingsDialog::setCurrentPage(int page)
{
    m_ui->tabWidget->setCurrentIndex(page);
}

void SettingsDialog::on_import_fullconfiguration()
{
    qDebug() << Q_FUNC_INFO;

    QFileDialog *importFileDialog = new QFileDialog(this, i18nc("import full configuration", "Import Full Configuration")
                                                    , QDir::homePath()
                                                    , QStringLiteral("latterc"));

    importFileDialog->setWindowIcon(QIcon::fromTheme("document-import"));
    importFileDialog->setLabelText(QFileDialog::Accept, i18nc("import full configuration","Import"));
    importFileDialog->setFileMode(QFileDialog::AnyFile);
    importFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    importFileDialog->setDefaultSuffix("latterc");

    QStringList filters;
    filters << QString(i18nc("import full configuration", "Latte Dock Full Configuration file") + "(*.latterc)");
    importFileDialog->setNameFilters(filters);

    connect(importFileDialog, &QFileDialog::finished, importFileDialog, &QFileDialog::deleteLater);

    connect(importFileDialog, &QFileDialog::fileSelected, this, [&](const QString & file) {
        Layouts::Importer::LatteFileVersion version = Layouts::Importer::fileVersion(file);
        qDebug() << "VERSION :::: " << version;

        if (version == Layouts::Importer::ConfigVersion2
                || version == Layouts::Importer::ConfigVersion1) {
            auto msg = new QMessageBox(this);
            msg->setIcon(QMessageBox::Warning);
            msg->setWindowTitle(i18n("Import: Full Configuration File"));
            msg->setText(i18n("You are importing full configuration file. Be careful, all <b>current settings and layouts will be lost</b>. It is advised to <b>take backup</b> first!<br>"));
            msg->setStandardButtons(QMessageBox::Cancel);

            QPushButton *takeBackupBtn = new QPushButton(msg);
            takeBackupBtn->setText(i18nc("export full configuration", "Take Backup..."));
            takeBackupBtn->setIcon(QIcon::fromTheme("document-export"));
            takeBackupBtn->setToolTip(i18n("Export your full configuration in order to take backup"));

            QPushButton *importBtn = new QPushButton(msg);
            importBtn->setText(i18nc("import full configuration", "Import"));
            importBtn->setIcon(QIcon::fromTheme("document-import"));
            importBtn->setToolTip(i18n("Import your full configuration and drop all your current settings and layouts"));

            msg->addButton(takeBackupBtn, QMessageBox::AcceptRole);
            msg->addButton(importBtn, QMessageBox::AcceptRole);
            msg->setDefaultButton(takeBackupBtn);

            connect(msg, &QFileDialog::finished, msg, &QFileDialog::deleteLater);

            connect(importBtn, &QPushButton::clicked, this, [&, file](bool check) {
                m_corona->importFullConfiguration(file);
            });

            connect(takeBackupBtn, &QPushButton::clicked, this, [&](bool check) {
                on_export_fullconfiguration();
            });

            msg->open();
        }
    });

    importFileDialog->open();
}

void SettingsDialog::on_export_fullconfiguration()
{
    //! Update ALL active original layouts before exporting,
    m_corona->layoutsManager()->synchronizer()->syncActiveLayoutsToOriginalFiles();

    QFileDialog *exportFileDialog = new QFileDialog(this, i18n("Export Full Configuration"),
                                                    QDir::homePath(),
                                                    QStringLiteral("latterc"));

    exportFileDialog->setLabelText(QFileDialog::Accept, i18nc("export full configuration","Export"));
    exportFileDialog->setFileMode(QFileDialog::AnyFile);
    exportFileDialog->setAcceptMode(QFileDialog::AcceptSave);
    exportFileDialog->setDefaultSuffix("latterc");

    QStringList filters;
    QString filter2(i18nc("export full configuration", "Latte Dock Full Configuration file v0.2") + "(*.latterc)");

    filters << filter2;

    exportFileDialog->setNameFilters(filters);

    connect(exportFileDialog, &QFileDialog::finished, exportFileDialog, &QFileDialog::deleteLater);

    connect(exportFileDialog, &QFileDialog::fileSelected, this, [&](const QString & file) {
        auto showExportConfigurationError = [this]() {
            showInlineMessage(i18n("Full configuration export <b>failed</b>..."), KMessageWidget::Error);
        };

        if (m_corona->layoutsManager()->importer()->exportFullConfiguration(file)) {
            QAction *openUrlAction = new QAction(i18n("Open Location..."), this);
            openUrlAction->setData(file);
            QList<QAction *> actions;
            actions << openUrlAction;

            connect(openUrlAction, &QAction::triggered, this, [&, openUrlAction]() {
                QString file = openUrlAction->data().toString();

                if (!file.isEmpty()) {
                    KIO::highlightInFileManager({file});
                }
            });

            showInlineMessage(i18n("Full configuration export succeeded..."),
                              KMessageWidget::Information,
                              SettingsDialog::INFORMATIONWITHACTIONINTERVAL,
                              actions);
        } else {
            showExportConfigurationError();
        }
    });

    exportFileDialog->open();

    QDate currentDate = QDate::currentDate();
    QString proposedName = QStringLiteral("Latte Dock (") + currentDate.toString("yyyy-MM-dd")+")";

    exportFileDialog->selectFile(proposedName);
}

void SettingsDialog::requestImagesDialog(int row)
{
    QStringList mimeTypeFilters;
    mimeTypeFilters << "image/jpeg" // will show "JPEG image (*.jpeg *.jpg)
                    << "image/png";  // will show "PNG image (*.png)"

    QFileDialog dialog(this);
    dialog.setMimeTypeFilters(mimeTypeFilters);

    QString background = "";// m_model->data(m_model->index(row, COLORCOLUMN), Qt::BackgroundRole).toString();

    if (background.startsWith("/") && QFileInfo(background).exists()) {
        dialog.setDirectory(QFileInfo(background).absolutePath());
        dialog.selectFile(background);
    }

    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();

        if (files.count() > 0) {
            // m_model->setData(m_model->index(row, COLORCOLUMN), files[0], Qt::BackgroundRole);
        }
    }
}

void SettingsDialog::requestColorsDialog(int row)
{
    /*QColorDialog dialog(this);
    QString textColor = m_model->data(m_model->index(row, Settings::Model::Layouts::BACKGROUNDCOLUMN), Qt::UserRole).toString();
    dialog.setCurrentColor(QColor(textColor));

    if (dialog.exec()) {
        qDebug() << dialog.selectedColor().name();
        m_model->setData(m_model->index(row, COLORCOLUMN), dialog.selectedColor().name(), Qt::UserRole);
    }*/
}

void SettingsDialog::accept()
{
    //! disable accept totally in order to avoid closing with ENTER key with no real reason
    qDebug() << Q_FUNC_INFO;
}

void SettingsDialog::apply()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_ui->buttonBox->button(QDialogButtonBox::Apply)->isEnabled()) {
        return;
    }

    save();
}

void SettingsDialog::reset()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_ui->buttonBox->button(QDialogButtonBox::Reset)->isEnabled()) {
        return;
    }

    if (m_ui->tabWidget->currentIndex() == Latte::Types::LayoutPage) {
        m_tabLayoutsHandler->reset();
    } else if (m_ui->tabWidget->currentIndex() == Latte::Types::PreferencesPage) {
        m_preferencesHandler->reset();
    }
}

void SettingsDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;

    if (m_ui->tabWidget->currentIndex() == Latte::Types::LayoutPage) {
        //! do nothing, should be disabled
    } else if (m_ui->tabWidget->currentIndex() == Latte::Types::PreferencesPage) {
        m_preferencesHandler->resetDefaults();
    }
}

void SettingsDialog::updateApplyButtonsState()
{
    bool changed{false};

    //! Ok, Apply Buttons

    if ((currentPage() == Latte::Types::LayoutPage && m_tabLayoutsHandler->dataAreChanged())
            ||(currentPage() == Latte::Types::PreferencesPage && m_preferencesHandler->dataAreChanged())) {
        changed = true;
    }

    if (changed) {
        m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
        m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(true);
    } else {
        m_ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        m_ui->buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    }

    //! RestoreDefaults Button
    if (m_ui->tabWidget->currentIndex() == Latte::Types::LayoutPage) {
        m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setVisible(false);
    } else if (m_ui->tabWidget->currentIndex() == Latte::Types::PreferencesPage) {
        m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setVisible(true);

        //! Defaults for general Latte settings
        if (m_preferencesHandler->inDefaultValues() ) {
            m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
        } else {
            m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(true);
        }
    }
}

void SettingsDialog::showLayoutInformation()
{
    /*  int currentRow = m_ui->layoutsView->currentIndex().row();

    QString id = m_model->data(m_model->index(currentRow, IDCOLUMN), Qt::DisplayRole).toString();
    QString name = m_model->data(m_model->index(currentRow, NAMECOLUMN), Qt::DisplayRole).toString();

    Layout::GenericLayout *genericActive= m_corona->layoutsManager()->synchronizer()->layout(o_layoutsOriginalData[id].originalName());
    Layout::GenericLayout *generic = genericActive ? genericActive : m_layouts[id];

    auto msg = new QMessageBox(this);
    msg->setWindowTitle(name);
    msg->setText(generic->reportHtml(m_corona->screenPool()));

    msg->open();*/
}

void SettingsDialog::showScreensInformation()
{
    /*  QList<int> assignedScreens;

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QString id = m_model->data(m_model->index(i, IDCOLUMN), Qt::DisplayRole).toString();
        QString name = m_model->data(m_model->index(i, NAMECOLUMN), Qt::DisplayRole).toString();

        Layout::GenericLayout *genericActive= m_corona->layoutsManager()->synchronizer()->layout(o_layoutsOriginalData[id].originalName());
        Layout::GenericLayout *generic = genericActive ? genericActive : m_layouts[id];

        QList<int> vScreens = generic->viewsScreens();

        for (const int scrId : vScreens) {
            if (!assignedScreens.contains(scrId)) {
                assignedScreens << scrId;
            }
        }
    }

    auto msg = new QMessageBox(this);
    msg->setWindowTitle(i18n("Screens Information"));
    msg->setText(m_corona->screenPool()->reportHtml(assignedScreens));

    msg->open();*/
}

void SettingsDialog::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void SettingsDialog::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urlList = event->mimeData()->urls();

        QStringList layoutNames;

        for (int i = 0; i < qMin(urlList.size(), 20); ++i) {
            QString layoutPath = urlList[i].path();

            if (layoutPath.endsWith(".layout.latte")) {
              //  Settings::Data::Layout importedlayout = m_layoutsController->addLayoutForFile(layoutPath);
              //  layoutNames << importedlayout.name;
            }
        }

        if (layoutNames.count() == 1) {
            showInlineMessage(i18nc("settings:layout imported successfully","Layout <b>%0</b> imported successfully...").arg(layoutNames[0]),
                    KMessageWidget::Information,
                    SettingsDialog::INFORMATIONINTERVAL);
        } else if (layoutNames.count() > 1) {
            showInlineMessage(i18nc("settings:layouts imported successfully","Layouts <b>%0</b> imported successfully...").arg(layoutNames.join(", )")),
                              KMessageWidget::Information,
                              SettingsDialog::INFORMATIONINTERVAL);
        }
    }
}

void SettingsDialog::keyPressEvent(QKeyEvent *event)
{
    if (event && event->key() == Qt::Key_Escape) {
        if (m_ui->messageWidget->isVisible()) {
            m_hideInlineMessageTimer.stop();
            m_ui->messageWidget->animatedHide();
            clearCurrentMessageActions();
            return;
        }
    }

    QDialog::keyPressEvent(event);
}

void SettingsDialog::keyReleaseEvent(QKeyEvent *event)
{
    if (event && currentPage() == Types::LayoutPage){
        m_tabLayoutsHandler->on_keyReleaseEvent(event);
    }

    QDialog::keyReleaseEvent(event);
}

void SettingsDialog::updateWindowActivities()
{
    if (KWindowSystem::isPlatformX11()) {
        KWindowSystem::setOnActivities(winId(), QStringList());
    }
}

void SettingsDialog::save()
{
    if (currentPage() == Latte::Types::LayoutPage) {
        m_tabLayoutsHandler->save();
    } else if (currentPage() == Latte::Types::PreferencesPage) {
        m_preferencesHandler->save();
    }
}

void SettingsDialog::clearCurrentMessageActions()
{
    while(m_currentMessageActions.count() > 0) {
        QAction *action = m_currentMessageActions.takeAt(0);
        m_ui->messageWidget->removeAction(action);
        action->deleteLater();
    }
}

void SettingsDialog::showInlineMessage(const QString &msg, const KMessageWidget::MessageType &type, const int &hideInterval, QList<QAction *> actions)
{
    if (msg.isEmpty()) {
        return;
    }

    if (!m_currentMessageActions.isEmpty()) {
        clearCurrentMessageActions();
    }

    m_currentMessageActions = actions;

    for (int i=0; i<actions.count(); ++i) {
        m_ui->messageWidget->addAction(actions[i]);
    }

    m_hideInlineMessageTimer.stop();

    if (m_ui->messageWidget->isVisible()) {
        m_ui->messageWidget->animatedHide();
    }

    m_ui->messageWidget->setText(msg);

    // TODO: wrap at arbitrary character positions once QLabel can do this
    // https://bugreports.qt.io/browse/QTBUG-1276
    m_ui->messageWidget->setWordWrap(true);
    m_ui->messageWidget->setMessageType(type);
    m_ui->messageWidget->setWordWrap(false);

    const int unwrappedWidth = m_ui->messageWidget->sizeHint().width();
    m_ui->messageWidget->setWordWrap(unwrappedWidth > size().width());

    m_ui->messageWidget->animatedShow();

    if (hideInterval > 0) {
        m_hideInlineMessageTimer.setInterval(hideInterval);
        m_hideInlineMessageTimer.start();
    }
}

}//end of namespace
