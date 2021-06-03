/*
    SPDX-FileCopyrightText: 2017 Smith AR <audoban@openmailbox.org>
    SPDX-FileCopyrightText: 2017 Michail Vourlakos <mvourlakos@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "settingsdialog.h"

// local
#include "ui_settingsdialog.h"
#include "../universalsettings.h"
#include "../generic/generictools.h"
#include "../screensdialog/screensdialog.h"
#include "../../lattecorona.h"
#include "../../screenpool.h"
#include "../../data/layoutdata.h"
#include "../../layout/centrallayout.h"
#include "../../layouts/importer.h"
#include "../../layouts/manager.h"
#include "../../layouts/synchronizer.h"
#include "../../plasma/extended/theme.h"

// Qt
#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>

// KDE
#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowSystem>
#include <KIO/OpenFileManagerWindowJob>


namespace Latte {
namespace Settings {
namespace Dialog {

SettingsDialog::SettingsDialog(QWidget *parent, Latte::Corona *corona)
    : GenericDialog(parent),
      m_ui(new Ui::SettingsDialog),
      m_corona(corona),
      m_storage(KConfigGroup(KSharedConfig::openConfig(),"LatteSettingsDialog"))
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAcceptDrops(true);
    m_ui->setupUi(this);

    //setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    //! load window size
    loadConfig();


    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::apply);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked,
            this, &SettingsDialog::reset);
    connect(m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
            this, &SettingsDialog::restoreDefaults);

    //! Global Menu
    initGlobalMenu();

    m_tabLayoutsHandler = new Settings::Handler::TabLayouts(this);
    m_tabPreferencesHandler = new Settings::Handler::TabPreferences(this);

    //! load settings after handlers in order to make migration process correctly
    //! and remove deprecated values totally from universalsettings
    loadConfig();
    resize(m_windowSize);

    m_ui->buttonBox->button(QDialogButtonBox::Apply)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    m_ui->buttonBox->button(QDialogButtonBox::Reset)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));

    //! SIGNALS
    connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &SettingsDialog::onCurrentTabChanged);

    connect(m_tabLayoutsHandler, &Settings::Handler::TabLayouts::dataChanged, this, &SettingsDialog::updateApplyButtonsState);

    connect(m_tabPreferencesHandler, &Settings::Handler::TabPreferences::dataChanged, this, &SettingsDialog::updateApplyButtonsState);
    connect(m_tabPreferencesHandler, &Settings::Handler::TabPreferences::borderlessMaximizedChanged,  this, [&]() {
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

    updateApplyButtonsState();
}

SettingsDialog::~SettingsDialog()
{
    qDebug() << Q_FUNC_INFO;

    setStoredWindowSize(size());
    saveConfig();
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
    connect(m_importFullAction, &QAction::triggered, this, &SettingsDialog::importFullConfiguration);

    m_exportFullAction = m_fileMenu->addAction(i18n("Export Configuration..."));
    m_exportFullAction->setIcon(QIcon::fromTheme("document-export"));
    m_exportFullAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_E));
    m_exportFullAction->setToolTip(i18n("Export your full configuration to create backup"));
    connect(m_exportFullAction, &QAction::triggered, this, &SettingsDialog::exportFullConfiguration);

    m_fileMenu->addSeparator();

    QAction *screensAction = m_fileMenu->addAction(i18n("&Screens..."));
    screensAction->setIcon(QIcon::fromTheme("document-properties"));
    screensAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_S));
    screensAction->setToolTip(i18n("Examing your screens and remove deprecated references"));
    connect(screensAction, &QAction::triggered, this, &SettingsDialog::showScreensDialog);

    QAction *quitAction = m_fileMenu->addAction(i18n("&Quit Latte"));
    quitAction->setIcon(QIcon::fromTheme("application-exit"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));


    //! triggers
    connect(quitAction, &QAction::triggered, this, [&]() {
        bool accepted = saveChanges();

        if (accepted) {
            close();
            m_corona->quitApplication();
        }
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

QSize SettingsDialog::storedWindowSize() const
{
    return m_windowSize;
}

void SettingsDialog::setStoredWindowSize(const QSize &size)
{
    if (m_windowSize == size) {
        return;
    }

    m_windowSize = size;
}

QSize SettingsDialog::downloadWindowSize() const
{
    return m_downloadWindowSize;
}

void SettingsDialog::setDownloadWindowSize(const QSize &size)
{
    if (m_downloadWindowSize == size) {
        return;
    }

    m_downloadWindowSize = size;
}


QMenuBar *SettingsDialog::appMenuBar() const
{
    return m_globalMenuBar;
}

QMenu *SettingsDialog::fileMenu() const
{
    return m_fileMenu;
}

QMenu *SettingsDialog::helpMenu() const
{
    return m_helpMenu->menu();
}

ConfigurationPage SettingsDialog::currentPage()
{
    ConfigurationPage cPage= static_cast<ConfigurationPage>(m_ui->tabWidget->currentIndex());

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

void SettingsDialog::importFullConfiguration()
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
        Latte::Layouts::Importer::LatteFileVersion version = Latte::Layouts::Importer::fileVersion(file);
        qDebug() << "VERSION :::: " << version;

        if (version == Latte::Layouts::Importer::ConfigVersion2
                || version == Latte::Layouts::Importer::ConfigVersion1) {
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
                exportFullConfiguration();
            });

            msg->open();
        }
    });

    importFileDialog->open();
}

void SettingsDialog::exportFullConfiguration()
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
            showInlineMessage(i18n("Full configuration export <b>failed</b>..."),
                              KMessageWidget::Error,
                              true);
        };

        if (m_corona->layoutsManager()->importer()->exportFullConfiguration(file)) {
            QAction *openUrlAction = new QAction(i18n("Open Location..."), this);
            openUrlAction->setIcon(QIcon::fromTheme("document-open"));
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
                              KMessageWidget::Positive,
                              false,
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

void SettingsDialog::showScreensDialog()
{
    auto screensDlg = new Settings::Dialog::ScreensDialog(this, m_tabLayoutsHandler->layoutsController());
    screensDlg->exec();
}

void SettingsDialog::accept()
{
    //! disable accept totally in order to avoid closing with ENTER key with no real reason
    qDebug() << Q_FUNC_INFO;
}

void SettingsDialog::reject()
{
    bool accepted = saveChanges();

    if (accepted) {
        QDialog::reject();
    }
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

    if (m_ui->tabWidget->currentIndex() == LayoutPage) {
        m_tabLayoutsHandler->reset();
    } else if (m_ui->tabWidget->currentIndex() == PreferencesPage) {
        m_tabPreferencesHandler->reset();
    }
}

void SettingsDialog::restoreDefaults()
{
    qDebug() << Q_FUNC_INFO;

    if (m_ui->tabWidget->currentIndex() == LayoutPage) {
        //! do nothing, should be disabled
    } else if (m_ui->tabWidget->currentIndex() == PreferencesPage) {
        m_tabPreferencesHandler->resetDefaults();
    }
}

void SettingsDialog::updateApplyButtonsState()
{
    bool changed{false};

    //! Ok, Apply Buttons

    if ((currentPage() == LayoutPage && m_tabLayoutsHandler->hasChangedData())
            ||(currentPage() == PreferencesPage && m_tabPreferencesHandler->hasChangedData())) {
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
    if (m_ui->tabWidget->currentIndex() == LayoutPage) {
        m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setVisible(false);
    } else if (m_ui->tabWidget->currentIndex() == PreferencesPage) {
        m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setVisible(true);

        //! Defaults for general Latte settings
        if (m_tabPreferencesHandler->inDefaultValues() ) {
            m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(false);
        } else {
            m_ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)->setEnabled(true);
        }
    }
}

bool SettingsDialog::saveChanges()
{
    if ((m_acceptedPage == LayoutPage && m_tabLayoutsHandler->hasChangedData())
        || (m_acceptedPage == PreferencesPage && m_tabPreferencesHandler->hasChangedData())) {

        QString tabName = m_ui->tabWidget->tabBar()->tabText(m_acceptedPage).remove("&");
        QString saveChangesText = i18n("The settings of <b>%1</b> tab have changed.<br/>Do you want to apply the changes or discard them?", tabName);

        KMessageBox::ButtonCode result = saveChangesConfirmation(saveChangesText);

        if (result == KMessageBox::Yes) {
            save();
        } else if (result == KMessageBox::No) {
            reset();
        } else {
            return false;
        }
    }

    return true;
}

void SettingsDialog::onCurrentTabChanged(int index)
{
    //! Before switching into a new tab the user must confirm first if the data should be saved or not

    if ((m_acceptedPage == LayoutPage && m_tabLayoutsHandler->hasChangedData())
        || (m_acceptedPage == PreferencesPage && m_tabPreferencesHandler->hasChangedData())) {

        if (index != m_acceptedPage) {
            m_nextPage = index;
            setCurrentPage(m_acceptedPage);
            return;
        }

        bool approvedNext = saveChanges();

        if (!approvedNext) {
            m_nextPage = m_acceptedPage;
            return;
        }
    } else {
        m_nextPage = index;
    }

    m_acceptedPage = m_nextPage >= 0 ? m_nextPage : index/*initialize*/;
    m_nextPage = m_acceptedPage;

    setCurrentPage(m_acceptedPage);
    updateApplyButtonsState();
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
    if (currentPage() == LayoutPage){
        m_tabLayoutsHandler->onDragEnterEvent(event);
    } else {
        QDialog::dragEnterEvent(event);
    }
}

void SettingsDialog::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (currentPage() == LayoutPage){
        m_tabLayoutsHandler->onDragLeaveEvent(event);
    } else {
        QDialog::dragLeaveEvent(event);
    }
}

void SettingsDialog::dragMoveEvent(QDragMoveEvent *event)
{
    if (currentPage() == LayoutPage){
        m_tabLayoutsHandler->onDragMoveEvent(event);
    } else {
        QDialog::dragMoveEvent(event);
    }
}

void SettingsDialog::dropEvent(QDropEvent *event)
{
    if (currentPage() == LayoutPage){
        m_tabLayoutsHandler->onDropEvent(event);
    } else {
        QDialog::dropEvent(event);
    }
}



void SettingsDialog::updateWindowActivities()
{
    if (KWindowSystem::isPlatformX11()) {
        KWindowSystem::setOnActivities(winId(), QStringList());
    }
}

void SettingsDialog::save()
{
    qDebug() << Q_FUNC_INFO;

    if (currentPage() == LayoutPage) {
        m_tabLayoutsHandler->save();
    } else if (currentPage() == PreferencesPage) {
        m_tabPreferencesHandler->save();
    }
}

void SettingsDialog::loadConfig()
{
    m_windowSize = m_storage.readEntry("windowSize", QSize(1100, 750));
    m_downloadWindowSize = m_storage.readEntry("downloadWindowSize", QSize(980, 600));
}

void SettingsDialog::saveConfig()
{
    m_storage.writeEntry("windowSize", m_windowSize);
    m_storage.writeEntry("downloadWindowSize", m_downloadWindowSize);
}

}
}
}//end of namespace
