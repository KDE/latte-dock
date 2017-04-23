#include "globalsettings.h"
#include "../liblattedock/extras.h"

#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QDesktopServices>

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

        connect(this, &GlobalSettings::importLayoutSignal, this, &GlobalSettings::importLayoutInternal);

        init();
    }
}

GlobalSettings::~GlobalSettings()
{
    m_altSessionAction->deleteLater();
    m_configGroup.sync();
    m_externalGroup.sync();
}

void GlobalSettings::init()
{
    //! check if user has set the autostart option
    bool autostartUserSet = m_configGroup.readEntry("userConfigureAutostart", false);

    if (!autostartUserSet && !autostart()) {
        setAutostart(true);
    }

    initExtConfiguration();
}

void GlobalSettings::initExtConfiguration()
{
    KSharedConfigPtr extConfig = KSharedConfig::openConfig(QDir::homePath() + "/.config/lattedockextrc");
    m_externalGroup = KConfigGroup(extConfig, "External");
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
        //! the first time that the user disables the autostart, this is recorded
        //! and from now own it will not be recreated it in the beginning
        if (!m_configGroup.readEntry("userConfigureAutostart", false)) {
            m_configGroup.writeEntry("userConfigureAutostart", true);
        }

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

    loadExtConfiguration();
}

void GlobalSettings::save()
{
    m_configGroup.writeEntry("exposeAltSession", m_exposeAltSession);
    m_configGroup.sync();
}

void GlobalSettings::loadExtConfiguration()
{
    //! load default layouts
    QVariantMap layout1;
    layout1.insert(QString("key"), QString(i18nc("default layout", "Default")));
    layout1.insert(QString("value"), QVariant(QString(m_corona->kPackage().filePath("layout1"))));

    QVariantMap layout2;
    layout2.insert(QString("key"), QString(i18nc("plasma layout", "Plasma")));
    layout2.insert(QString("value"), QVariant(QString(m_corona->kPackage().filePath("layout2"))));

    QVariantMap layout3;
    layout3.insert(QString("key"), QString(i18nc("unity layout", "Unity")));
    layout3.insert(QString("value"), QVariant(QString(m_corona->kPackage().filePath("layout3"))));

    QVariantMap layout4;
    layout4.insert(QString("key"), QString(i18nc("extended layout", "Extended")));
    layout4.insert(QString("value"), QVariant(QString(m_corona->kPackage().filePath("layout4"))));

    m_defaultLayouts.append(layout1);
    m_defaultLayouts.append(layout2);
    m_defaultLayouts.append(layout3);
    m_defaultLayouts.append(layout4);

    //! load user layouts
    QStringList userLayouts = m_externalGroup.readEntry("userLayouts", QStringList());
    QStringList confirmedLayouts;

    foreach (QString layout, userLayouts) {
        QFile layoutFile(layout);

        if (layoutFile.exists() && !confirmedLayouts.contains(layout)) {
            confirmedLayouts.append(layout);

            QVariantMap userLayout;
            int p1 = layout.lastIndexOf("/");
            int p2 = layout.lastIndexOf(".");
            //!add the filename as a key
            userLayout.insert(QString("key"), layout.mid(p1 + 1, p2 - p1 - 1));
            userLayout.insert(QString("value"), QVariant(QString(layout)));

            m_userLayouts.append(userLayout);
            m_userLayoutsFiles.append(layout);
        }
    }

    //! a save is needed because on first loading we check also if any of the user layout files
    //! has been removed by the user
    saveExtConfiguration();
}

void GlobalSettings::saveExtConfiguration()
{
    m_externalGroup.writeEntry("userLayouts", m_userLayoutsFiles);
    m_externalGroup.sync();
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

        foreach (auto &name, rootDir->entries()) {
            auto fileEntry = rootDir->file(name);

            if (fileEntry && (fileEntry->name() == "lattedockrc"
                              || fileEntry->name() == "lattedock-appletsrc")) {
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
    , this, [&](const QString & file) {
        importLayoutInternal(file);
    });

    m_fileDialog->open();
}

void GlobalSettings::importLayout(const QString &name, const QString &file)
{
    qDebug() << "layout should be imported : " << file;

    auto msg = new QMessageBox();
    //msg->setIcon(QMessageBox::Warning);
    msg->setWindowTitle(i18n("Activate Layout"));
    msg->setText(i18n("You are going to activate a layout called <b>%1</b>, <br>by doing so the current layout will be lost... <br>Do you want to proceed?").arg(name));
    msg->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msg->setDefaultButton(QMessageBox::Cancel);

    connect(msg, &QMessageBox::finished, this, [&, file](int result){
        if (result == QMessageBox::Ok)
            importLayoutInternal(file);
    });
    connect(msg, &QMessageBox::finished, msg, &QMessageBox::deleteLater);
    msg->open();
}

void GlobalSettings::importLayoutInternal(const QString &file)
{
    auto showMsgError = [&]() {
        auto msg = new QMessageBox;
        msg->setText(i18nc("import/export config", "The file has a wrong format, do you want open other file?"));
        msg->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        connect(msg, &QMessageBox::finished, this, [&, file](int result){
            if (result == QMessageBox::Ok)
                importConfiguration();
        });
        connect(msg, &QMessageBox::finished, msg, &QMessageBox::deleteLater);

        msg->open();
    };

    if (!QFile::exists(file)) {
        showMsgError();
        return;
    }

    //! start:: update the user layouts

    //! first check if this is a default layout, in that case it shouldnt be added
    bool defaultLayout = false;

    foreach (QVariant it, m_userLayouts) {
        if (it.canConvert<QVariantMap>()) {
            QVariantMap map = it.toMap();

            if (map["value"].toString() == file) {
                defaultLayout = true;
                break;
            }
        }
    }

    if (!defaultLayout) {
        if (m_userLayoutsFiles.contains(file)) {
            m_userLayoutsFiles.removeAll(file);
        }

        m_userLayoutsFiles.prepend(file);
        saveExtConfiguration();
    }

    //! end:: update the user layouts

    KTar archive(file, QStringLiteral("application/x-tar"));
    archive.open(QIODevice::ReadOnly);

    if (!archive.isOpen()) {
        showMsgError();
        return;
    }

    auto rootDir = archive.directory();

    if (rootDir) {
        foreach (auto &name, rootDir->entries()) {
            auto fileEntry = rootDir->file(name);

            if (fileEntry && (fileEntry->name() == "lattedockrc"
                              || fileEntry->name() == "lattedock-appletsrc")) {
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
    , this, [&](const QString & file) {
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
        , this, [file]() {
            QDesktopServices::openUrl({QFileInfo(file).canonicalPath()});
        });

        notification->sendEvent();
    });

    m_fileDialog->open();
}

QVariantList GlobalSettings::layouts()
{
    QVariantList result;
    result.append(m_defaultLayouts);

    //! clean up the user styles first in case some of them has been deleted from
    //! the filesystem
    foreach (QString layout, m_userLayoutsFiles) {
        QFile layoutFile(layout);

        if (!layoutFile.exists()) {
            m_userLayoutsFiles.removeAll(layout);

            foreach (QVariant it, m_userLayouts) {
                if (it.canConvert<QVariantMap>()) {
                    QVariantMap map = it.toMap();

                    if (map["value"].toString() == layout) {
                        m_userLayouts.removeAll(it);
                    }
                }
            }
        }
    }

    if (m_userLayouts.size() > 0) {
        QVariantMap emptyRecord;
        emptyRecord.insert(QString("key"), QString("-----"));
        emptyRecord.insert(QString("value"), QVariant(QString("")));
        result.append(emptyRecord);

        result.append(m_userLayouts);
    }

    return result;
}

}

#include "moc_globalsettings.cpp"
