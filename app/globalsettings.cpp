#include "globalsettings.h"
#include "../liblattedock/extras.h"

#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrent>

#include <KLocalizedString>
#include <KConfig>
#include <KArchive/KTar>
#include <KArchive/KArchiveEntry>
#include <KArchive/KArchiveDirectory>
#include <KNotification>

namespace Latte {

GlobalSettings::GlobalSettings(QObject *parent)
    : QObject(parent)
{
    m_corona = qobject_cast<DockCorona *>(parent);

    if (m_corona) {
        m_configGroup = m_corona->config()->group("General");
        //! create the alternative session action
        const QIcon altIcon = QIcon::fromTheme("user-identity");
        m_altSessionAction = new QAction(altIcon, i18n("Alternative Session"), this);
        m_altSessionAction->setStatusTip(i18n("Enable/Disable Alternative Session"));
        m_altSessionAction->setCheckable(true);
        connect(m_altSessionAction, &QAction::triggered, this, &GlobalSettings::enableAltSession);
        connect(m_corona, &DockCorona::currentSessionChanged, this, &GlobalSettings::currentSessionChangedSlot);
    }
}

GlobalSettings::~GlobalSettings()
{
    m_altSessionAction->deleteLater();
    m_configGroup.sync();
}

void GlobalSettings::enableAltSession(bool enabled)
{
    if (enabled) {
        m_corona->switchToSession(Dock::AlternativeSession);
    } else {
        m_corona->switchToSession(Dock::DefaultSession);
    }
}

bool GlobalSettings::exposeAltSession() const
{
    return m_exposeAltSession;
}

void GlobalSettings::setExposeAltSession(bool state)
{
    if (m_exposeAltSession == state) {
        return;
    }

    m_exposeAltSession = state;
    save();
    emit exposeAltSessionChanged();
}

void GlobalSettings::currentSessionChangedSlot(Dock::SessionType type)
{
    if (m_corona->currentSession() == Dock::DefaultSession)
        m_altSessionAction->setChecked(false);
    else
        m_altSessionAction->setChecked(true);

    emit currentSessionChanged();
}

QAction *GlobalSettings::altSessionAction() const
{
    return m_altSessionAction;
}

bool GlobalSettings::autostart() const
{
    QFile autostartFile(QDir::homePath() + "/.config/autostart/latte-dock.desktop");
    return autostartFile.exists();
}

void GlobalSettings::setAutostart(bool state)
{
    QFile autostartFile(QDir::homePath() + "/.config/autostart/latte-dock.desktop");
    QFile metaFile("/usr/share/applications/latte-dock.desktop");

    if (!state && autostartFile.exists()) {
        autostartFile.remove();
        emit autostartChanged();
    } else if (state && metaFile.exists()) {
        metaFile.copy(autostartFile.fileName());
        //! I havent added the flag "OnlyShowIn=KDE;" into the autostart file
        //! because I fall onto a Plasma 5.8 case that this flag
        //! didnt let the plasma desktop to start
        emit autostartChanged();
    }
}

Dock::SessionType GlobalSettings::currentSession() const
{
    return m_corona->currentSession();
}

void GlobalSettings::setCurrentSession(Dock::SessionType session)
{
    if (currentSession() != session) {
        m_corona->switchToSession(session);
    }
}


//!BEGIN configuration functions
void GlobalSettings::load()
{
    setExposeAltSession(m_configGroup.readEntry("exposeAltSession", false));
}

void GlobalSettings::save()
{
    m_configGroup.writeEntry("exposeAltSession", m_exposeAltSession);
    m_configGroup.sync();
}
//!END configuration functions

bool GlobalSettings::importHelper(const QString &fileName)
{
    if (!QFile::exists(fileName))
        return false;

    KTar archive(fileName, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    if (!archive.isOpen())
        return false;

    const auto rootDir = archive.directory();
    QDir tempDir {QDir::tempPath() + "/latterc-unconmpressed"};

    auto clean = [&]() {
        if (tempDir.exists()) {
            tempDir.removeRecursively();
        }
        archive.close();
    };

    auto showNotificationErr = []() {
        auto notification = new KNotification("import-fail", KNotification::CloseOnTimeout);
        notification->setText(i18nc("import/export config", "Failed to import configuration"));
        notification->sendEvent();
    };

    if (rootDir) {
        if (!tempDir.exists())
            tempDir.mkpath(tempDir.absolutePath());

        foreach(auto &name, rootDir->entries()) {
            auto fileEntry = rootDir->file(name);

            if (fileEntry && (fileEntry->name() == "lattedockrc"
                              || fileEntry->name() == "lattedock-appletsrc"))
            {
                if (!fileEntry->copyTo(tempDir.absolutePath())) {
                    clean();
                    showNotificationErr();
                    return false;
                }
            } else {
                qInfo() << i18nc("import/export config", "The file has a wrong format!!!");
                clean();
                showNotificationErr();
                return false;
            }
        }
    }

    const auto latterc = QDir::homePath() + "/.config/lattedockrc";
    const auto appletsrc = QDir::homePath() + "/.config/lattedock-appletsrc";

    // NOTE: I'm trying to avoid possible loss of information
    qInfo() << "Backing up old configuration files...";
    auto n = QString::number(qrand() % 256);
    QFile::copy(latterc, latterc + "." + n + ".bak");
    QFile::copy(appletsrc, appletsrc + "." + n + ".bak");

    qInfo() << "Importing the new configuration...";
    if (QFile::remove(latterc) && QFile::remove(appletsrc)) {
        QFile::copy(tempDir.absolutePath() + "/lattedockrc" , latterc);
        QFile::copy(tempDir.absolutePath() + "/lattedock-appletsrc", appletsrc);
    } else {
        showNotificationErr();
        return false;
    }

    clean();
    auto notification = new KNotification("import-done", KNotification::CloseOnTimeout);
    notification->setText(i18nc("import/export config", "Configuration imported successfully"));
    notification->sendEvent();

    return true;
}
void GlobalSettings::importConfiguration()
{
    if (m_fileDialog) {
        m_fileDialog->close();
        m_fileDialog->deleteLater();
    }

    m_fileDialog = new QFileDialog(nullptr, i18nc("import/export config", "Import configuration")
                                   , QDir::homePath()
                                   , QStringLiteral("latteconf"));

    m_fileDialog->setFileMode(QFileDialog::AnyFile);
    m_fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    m_fileDialog->setDefaultSuffix("latteconf");
    m_fileDialog->setNameFilter(i18nc("import/export config", "Latte Dock configuration file")
                                + "(*.latterc)");

    connect(m_fileDialog.data(), &QFileDialog::finished
    , m_fileDialog.data(), &QFileDialog::deleteLater);

    connect(m_fileDialog.data(), &QFileDialog::fileSelected
    , this, [&](const QString &file) {

        auto showMsgError = [&]() {
            auto msg = new QMessageBox;
            msg->setText(i18nc("import/export config", "The file has a wrong format"));
            msg->setDetailedText(i18nc("import/export config", "Do you want to open other file?"));
            msg->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

            connect(msg, &QMessageBox::accepted, this, &GlobalSettings::importConfiguration);
            connect(msg, &QMessageBox::finished, msg, &QMessageBox::deleteLater);

            msg->open();
        };

        if (!QFile::exists(file)) {
            showMsgError();
            return;
        }

        KTar archive(file, QStringLiteral("application/x-tar"));
        archive.open(QIODevice::ReadOnly);

        if (!archive.isOpen()) {
            showMsgError();
            return;
        }

        auto rootDir = archive.directory();

        if (rootDir) {
            foreach(auto &name, rootDir->entries()) {
                auto fileEntry = rootDir->file(name);

                if (fileEntry && (fileEntry->name() == "lattedockrc"
                                  || fileEntry->name() == "lattedock-appletsrc"))
                {
                    continue;
                } else {
                    archive.close();
                    showMsgError();
                    return;
                }
            }
        }

        archive.close();
        //NOTE: Restart latte for import the new configuration
        QProcess::startDetached(qGuiApp->applicationFilePath() + " --import \"" + file + "\"");
        qGuiApp->exit();

    });

    m_fileDialog->open();
}

void GlobalSettings::exportConfiguration()
{
    if (m_fileDialog) {
        m_fileDialog->close();
        m_fileDialog->deleteLater();
    }

    m_fileDialog = new QFileDialog(nullptr, i18nc("import/export config", "Export configuration")
                                   , QDir::homePath()
                                   , QStringLiteral("latterc"));
    m_fileDialog->setFileMode(QFileDialog::AnyFile);
    m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
    m_fileDialog->setDefaultSuffix("latterc");
    m_fileDialog->setNameFilter(i18nc("import/export config", "Latte Dock configuration file")
                                + "(*.latterc)");

    connect(m_fileDialog.data(), &QFileDialog::finished
    , m_fileDialog.data(), &QFileDialog::deleteLater);

    connect(m_fileDialog.data(), &QFileDialog::fileSelected
    , this, [&](const QString &file) {
        auto showNotificationError = []() {
            auto notification = new KNotification("export-fail", KNotification::CloseOnTimeout);
            notification->setText(i18nc("import/export config", "Failed to export configuration"));
            notification->sendEvent();
        };

        if (QFile::exists(file) && !QFile::remove(file)) {
            showNotificationError();
            return;
        }

        KTar archive(file, QStringLiteral("application/x-tar"));

        if (!archive.open(QIODevice::WriteOnly)) {
            showNotificationError();
            return;
        }

        std::unique_ptr<KConfig> config {m_corona->config()->copyTo(QDir::tempPath() + "/lattedock-appletsrc")};
        std::unique_ptr<KConfig> configApp {KSharedConfig::openConfig()->copyTo(QDir::tempPath() + "/lattedockrc")};

        config->sync();
        configApp->sync();

        archive.addLocalFile(config->name(), QStringLiteral("lattedock-appletsrc"));
        archive.addLocalFile(configApp->name(), QStringLiteral("lattedockrc"));
        archive.close();

        QFile::remove(config->name());
        QFile::remove(configApp->name());

        //NOTE: The pointer is automatically deleted when the event is closed
        auto notification = new KNotification("export-done", KNotification::CloseOnTimeout);
        notification->setActions({i18nc("import/export config", "Open location")});
        notification->setText(i18nc("import/export config", "Configuration exported successfully"));

        connect(notification, &KNotification::action1Activated
        , this, [&file]() {
            QDir path(file);
            path.cdUp();
            QDesktopServices::openUrl({path.absolutePath()});
        });

        notification->sendEvent();
    });

    m_fileDialog->open();
}
}

#include "moc_globalsettings.cpp"
