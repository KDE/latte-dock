/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein.org>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "tasktools.h"
#include <config-latte.h>

#include <KActivities/ResourceInstance>
#include <KApplicationTrader>
#include <KConfigGroup>
#include <KDesktopFile>
#include <kemailsettings.h>
#include <KMimeTypeTrader>
#include <KServiceTypeTrader>
#include <KSharedConfig>
#include <KStartupInfo>
#include <KWindowSystem>
#include <KProcessList>

#include <QDir>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QScreen>
#include <QUrlQuery>
#if HAVE_X11
#include <QX11Info>
#endif

namespace Latte
{
namespace WindowSystem
{

AppData appDataFromUrl(const QUrl &url, const QIcon &fallbackIcon)
{
    AppData data;
    data.url = url;

    if (url.hasQuery()) {
        QUrlQuery uQuery(url);

        if (uQuery.hasQueryItem(QLatin1String("iconData"))) {
            QString iconData(uQuery.queryItemValue(QLatin1String("iconData")));
            QPixmap pixmap;
            QByteArray bytes = QByteArray::fromBase64(iconData.toLocal8Bit(), QByteArray::Base64UrlEncoding);
            pixmap.loadFromData(bytes);
            data.icon.addPixmap(pixmap);
        }

        if (uQuery.hasQueryItem(QLatin1String("skipTaskbar"))) {
            QString skipTaskbar(uQuery.queryItemValue(QLatin1String("skipTaskbar")));
            data.skipTaskbar = (skipTaskbar == QLatin1String("true"));
        }
    }

    // applications: URLs are used to refer to applications by their KService::menuId
    // (i.e. .desktop file name) rather than the absolute path to a .desktop file.
    if (url.scheme() == QLatin1String("applications")) {
        const KService::Ptr service = KService::serviceByMenuId(url.path());

        if (service && url.path() == service->menuId()) {
            data.name = service->name();
            data.genericName = service->genericName();
            data.id = service->storageId();

            if (data.icon.isNull()) {
                data.icon = QIcon::fromTheme(service->icon());
            }
        }
    }

    if (url.isLocalFile() && KDesktopFile::isDesktopFile(url.toLocalFile())) {
        const KService::Ptr service = KService::serviceByStorageId(url.fileName());

        // Resolve to non-absolute menuId-based URL if possible.
        if (service) {
            const QString &menuId = service->menuId();

            if (!menuId.isEmpty()) {
                data.url = QUrl(QLatin1String("applications:") + menuId);
            }
        }

        if (service && QUrl::fromLocalFile(service->entryPath()) == url) {
            data.name = service->name();
            data.genericName = service->genericName();
            data.id = service->storageId();

            if (data.icon.isNull()) {
                data.icon = QIcon::fromTheme(service->icon());
            }
        } else {
            KDesktopFile f(url.toLocalFile());
            if (f.tryExec()) {
                data.name = f.readName();
                data.genericName = f.readGenericName();
                data.id = QUrl::fromLocalFile(f.fileName()).fileName();

                if (data.icon.isNull()) {
                    data.icon = QIcon::fromTheme(f.readIcon());
                }
            }
        }

        if (data.id.endsWith(".desktop")) {
            data.id = data.id.left(data.id.length() - 8);
        }
    } else if (url.scheme() == QLatin1String("preferred")) {
        data.id = defaultApplication(url);

        const KService::Ptr service = KService::serviceByStorageId(data.id);

        if (service) {
            const QString &menuId = service->menuId();
            const QString &desktopFile = service->entryPath();

            data.name = service->name();
            data.genericName = service->genericName();
            data.id = service->storageId();

            if (data.icon.isNull()) {
                data.icon = QIcon::fromTheme(service->icon());
            }

            // Update with resolved URL.
            if (!menuId.isEmpty()) {
                data.url = QUrl(QLatin1String("applications:") + menuId);
            } else {
                data.url = QUrl::fromLocalFile(desktopFile);
            }
        }
    }

    if (data.name.isEmpty()) {
        data.name = url.fileName();
    }

    if (data.icon.isNull()) {
        data.icon = fallbackIcon;
    }

    return data;
}

QUrl windowUrlFromMetadata(const QString &appId, quint32 pid,
    KSharedConfig::Ptr rulesConfig, const QString &xWindowsWMClassName)
{
    if (!rulesConfig) {
        return QUrl();
    }

    QUrl url;
    KService::List services;
    bool triedPid = false;

    // The code below this function goes on a hunt for services based on the metadata
    // that has been passed in. Occasionally, it will find more than one matching
    // service. In some scenarios (e.g. multiple identically-named .desktop files)
    // there's a need to pick the most useful one. The function below promises to "sort"
    // a list of services by how closely their KService::menuId() relates to the key that
    // has been passed in. The current naive implementation simply looks for a menuId
    // that starts with the key, prepends it to the list and returns it. In practice,
    // that means a KService with a menuId matching the appId will win over one with a
    // menuId that encodes a subfolder hierarchy.
    // A concrete example: Valve's Steam client is sometimes installed two times, once
    // natively as a Linux application, once via Wine. Both have .desktop files named
    // (S|)steam.desktop. The Linux native version is located in the menu by means of
    // categorization ("Games") and just has a menuId() matching the .desktop file name,
    // but the Wine version is placed in a folder hierarchy by Wine and gets a menuId()
    // of wine-Programs-Steam-Steam.desktop. The weighing done by this function makes
    // sure the Linux native version gets mapped to the former, while other heuristics
    // map the Wine version reliably to the latter.
    // In lieu of this weighing we just used whatever KServiceTypeTrader returned first,
    // so what we do here can be no worse.
    auto sortServicesByMenuId = [](KService::List &services, const QString &key) {
        if (services.count() == 1) {
            return;
        }

        for (const auto &service : services) {
            if (service->menuId().startsWith(key, Qt::CaseInsensitive)) {
                services.prepend(service);
                return;
            }
        }
    };

    if (!(appId.isEmpty() && xWindowsWMClassName.isEmpty())) {
        // Check to see if this wmClass matched a saved one ...
        KConfigGroup grp(rulesConfig, "Mapping");
        KConfigGroup set(rulesConfig, "Settings");

        // Evaluate MatchCommandLineFirst directives from config first.
        // Some apps have different launchers depending upon command line ...
        QStringList matchCommandLineFirst = set.readEntry("MatchCommandLineFirst", QStringList());

        if (!appId.isEmpty() && matchCommandLineFirst.contains(appId)) {
            triedPid = true;
            services = servicesFromPid(pid, rulesConfig);
        }

        // Try to match using xWindowsWMClassName also.
        if (!xWindowsWMClassName.isEmpty() && matchCommandLineFirst.contains("::" + xWindowsWMClassName)) {
            triedPid = true;
            services = servicesFromPid(pid, rulesConfig);
        }

        if (!appId.isEmpty()) {
            // Evaluate any mapping rules that map to a specific .desktop file.
            QString mapped(grp.readEntry(appId + "::" + xWindowsWMClassName, QString()));

            if (mapped.endsWith(QLatin1String(".desktop"))) {
                url = QUrl(mapped);
                return url;
            }

            if (mapped.isEmpty()) {
                mapped = grp.readEntry(appId, QString());

                if (mapped.endsWith(QLatin1String(".desktop"))) {
                    url = QUrl(mapped);
                    return url;
                }
            }

            // Some apps, such as Wine, cannot use xWindowsWMClassName to map to launcher name - as Wine itself is not a GUI app
            // So, Settings/ManualOnly lists window classes where the user will always have to manualy set the launcher ...
            QStringList manualOnly = set.readEntry("ManualOnly", QStringList());

            if (!appId.isEmpty() && manualOnly.contains(appId)) {
                return url;
            }

            // Try matching both appId and xWindowsWMClassName against StartupWMClass.
            // We do this before evaluating the mapping rules further, because StartupWMClass
            // is essentially a mapping rule, and we expect it to be set deliberately and
            // sensibly to instruct us what to do. Also, mapping rules
            //
            // StartupWMClass=STRING
            //
            //   If true, it is KNOWN that the application will map at least one
            //   window with the given string as its WM class or WM name hint.
            //
            // Source: https://specifications.freedesktop.org/startup-notification-spec/startup-notification-0.1.txt
            if (services.isEmpty()) {
                services =
                    KServiceTypeTrader::self()->query(QStringLiteral("Application"), QStringLiteral("exist Exec and ('%1' =~ StartupWMClass)").arg(appId));
                sortServicesByMenuId(services, appId);
            }

            if (services.isEmpty() && !xWindowsWMClassName.isEmpty()) {
                services = KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                             QStringLiteral("exist Exec and ('%1' =~ StartupWMClass)").arg(xWindowsWMClassName));
                sortServicesByMenuId(services, xWindowsWMClassName);
            }

            // Evaluate rewrite rules from config.
            if (services.isEmpty()) {
                KConfigGroup rewriteRulesGroup(rulesConfig, QStringLiteral("Rewrite Rules"));
                if (rewriteRulesGroup.hasGroup(appId)) {
                    KConfigGroup rewriteGroup(&rewriteRulesGroup, appId);

                    const QStringList &rules = rewriteGroup.groupList();
                    for (const QString &rule : rules) {
                        KConfigGroup ruleGroup(&rewriteGroup, rule);

                        const QString propertyConfig = ruleGroup.readEntry(QStringLiteral("Property"), QString());

                        QString matchProperty;
                        if (propertyConfig == QLatin1String("ClassClass")) {
                            matchProperty = appId;
                        } else if (propertyConfig == QLatin1String("ClassName")) {
                            matchProperty = xWindowsWMClassName;
                        }

                        if (matchProperty.isEmpty()) {
                            continue;
                        }

                        const QString serviceSearchIdentifier = ruleGroup.readEntry(QStringLiteral("Identifier"), QString());
                        if (serviceSearchIdentifier.isEmpty()) {
                            continue;
                        }

                        QRegularExpression regExp(ruleGroup.readEntry(QStringLiteral("Match")));
                        const auto match = regExp.match(matchProperty);

                        if (match.hasMatch()) {
                            const QString actualMatch = match.captured(QStringLiteral("match"));
                            if (actualMatch.isEmpty()) {
                                continue;
                            }

                            QString rewrittenString = ruleGroup.readEntry(QStringLiteral("Target")).arg(actualMatch);
                            // If no "Target" is provided, instead assume the matched property (appId/xWindowsWMClassName).
                            if (rewrittenString.isEmpty()) {
                                rewrittenString = matchProperty;
                            }

                            services =
                                KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                                  QStringLiteral("exist Exec and ('%1' =~ %2)").arg(rewrittenString, serviceSearchIdentifier));
                            sortServicesByMenuId(services, serviceSearchIdentifier);

                            if (!services.isEmpty()) {
                                break;
                            }
                        }
                    }
                }
            }

            // The appId looks like a path.
            if (services.isEmpty() && appId.startsWith(QLatin1String("/"))) {
                // Check if it's a path to a .desktop file.
                if (KDesktopFile::isDesktopFile(appId) && QFile::exists(appId)) {
                    return QUrl::fromLocalFile(appId);
                }

                // Check if the appId passes as a .desktop file path if we add the extension.
                const QString appIdPlusExtension(appId + QStringLiteral(".desktop"));

                if (KDesktopFile::isDesktopFile(appIdPlusExtension) && QFile::exists(appIdPlusExtension)) {
                    return QUrl::fromLocalFile(appIdPlusExtension);
                }
            }

            // Try matching mapped name against DesktopEntryName.
            if (!mapped.isEmpty() && services.isEmpty()) {
                services = KServiceTypeTrader::self()->query(
                    QStringLiteral("Application"),
                    QStringLiteral("exist Exec and ('%1' =~ DesktopEntryName) and (not exist NoDisplay or not NoDisplay)").arg(mapped));
                sortServicesByMenuId(services, mapped);
            }

            // Try matching mapped name against 'Name'.
            if (!mapped.isEmpty() && services.isEmpty()) {
                services =
                    KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                      QStringLiteral("exist Exec and ('%1' =~ Name) and (not exist NoDisplay or not NoDisplay)").arg(mapped));
                sortServicesByMenuId(services, mapped);
            }

            // Try matching appId against DesktopEntryName.
            if (services.isEmpty()) {
                services = KServiceTypeTrader::self()->query(
                    QStringLiteral("Application"),
                    QStringLiteral("exist Exec and ('%1' =~ DesktopEntryName)").arg(appId));
                sortServicesByMenuId(services, appId);
            }

            // Try matching appId against 'Name'.
            // This has a shaky chance of success as appId is untranslated, but 'Name' may be localized.
            if (services.isEmpty()) {
                services =
                    KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                      QStringLiteral("exist Exec and ('%1' =~ Name) and (not exist NoDisplay or not NoDisplay)").arg(appId));
                sortServicesByMenuId(services, appId);
            }

            // Check rules configuration for whether we want to hide this task.
            // Some window tasks update from bogus to useful metadata early during startup.
            // This config key allows listing the bogus metadata, and the matching window
            // tasks are hidden until they perform a metadate update that stops them from
            // matching.
            QStringList skipTaskbar = set.readEntry("SkipTaskbar", QStringList());

            if (skipTaskbar.contains(appId)) {
                QUrlQuery query(url);
                query.addQueryItem(QStringLiteral("skipTaskbar"), QStringLiteral("true"));
                url.setQuery(query);
            } else if (skipTaskbar.contains(mapped)) {
                QUrlQuery query(url);
                query.addQueryItem(QStringLiteral("skipTaskbar"), QStringLiteral("true"));
                url.setQuery(query);
            }
        }

        // Ok, absolute *last* chance, try matching via pid (but only if we have not already tried this!) ...
        if (services.isEmpty() && !triedPid) {
            services = servicesFromPid(pid, rulesConfig);
        }
    }

    // Try to improve on a possible from-binary fallback.
    // If no services were found or we got a fake-service back from getServicesViaPid()
    // we attempt to improve on this by adding a loosely matched reverse-domain-name
    // DesktopEntryName. Namely anything that is '*.appId.desktop' would qualify here.
    //
    // Illustrative example of a case where the above heuristics would fail to produce
    // a reasonable result:
    // - org.kde.dragonplayer.desktop
    // - binary is 'dragon'
    // - qapp appname and thus appId is 'dragonplayer'
    // - appId cannot directly match the desktop file because of RDN
    // - appId also cannot match the binary because of name mismatch
    // - in the following code *.appId can match org.kde.dragonplayer though
    if (services.isEmpty() || services.at(0)->desktopEntryName().isEmpty()) {
        auto matchingServices =
            KServiceTypeTrader::self()->query(QStringLiteral("Application"), QStringLiteral("exist Exec and ('%1' ~~ DesktopEntryName)").arg(appId));
        QMutableListIterator<KService::Ptr> it(matchingServices);
        while (it.hasNext()) {
            auto service = it.next();
            if (!service->desktopEntryName().endsWith("." + appId)) {
                it.remove();
            }
        }
        // Exactly one match is expected, otherwise we discard the results as to reduce
        // the likelihood of false-positive mappings. Since we essentially eliminate the
        // uniqueness that RDN is meant to bring to the table we could potentially end
        // up with more than one match here.
        if (matchingServices.length() == 1) {
            services = matchingServices;
        }
    }

    if (!services.isEmpty()) {
        const QString &menuId = services.at(0)->menuId();

        // applications: URLs are used to refer to applications by their KService::menuId
        // (i.e. .desktop file name) rather than the absolute path to a .desktop file.
        if (!menuId.isEmpty()) {
            url.setUrl(QStringLiteral("applications:") + menuId);
            return url;
        }

        QString path = services.at(0)->entryPath();

        if (path.isEmpty()) {
            path = services.at(0)->exec();
        }

        if (!path.isEmpty()) {
            QString query = url.query();
            url = QUrl::fromLocalFile(path);
            url.setQuery(query);
            return url;
        }
    }

    return url;
}

KService::List servicesFromPid(quint32 pid, KSharedConfig::Ptr rulesConfig)
{
    if (pid == 0) {
        return KService::List();
    }

    if (!rulesConfig) {
        return KService::List();
    }

    // Read the BAMF_DESKTOP_FILE_HINT environment variable which contains the actual desktop file path for Snaps.
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray bamfDesktopFileHint = QByteArrayLiteral("BAMF_DESKTOP_FILE_HINT");

        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }

            const QByteArray key = line.left(equalsIdx);
            if (key == bamfDesktopFileHint) {
                const QByteArray value = line.mid(equalsIdx + 1);

                KService::Ptr service = KService::serviceByDesktopPath(QString::fromUtf8(value));
                if (service) {
                    return {service};
                }
                break;
            }
        }
    }

    auto proc = KProcessList::processInfo(pid);
    if (!proc.isValid()) {
        return KService::List();
    }

    const QString cmdLine = proc.command();

    if (cmdLine.isEmpty()) {
        return KService::List();
    }

    return servicesFromCmdLine(cmdLine, proc.name(), rulesConfig);
}

KService::List servicesFromCmdLine(const QString &_cmdLine, const QString &processName,
    KSharedConfig::Ptr rulesConfig)
{
    QString cmdLine = _cmdLine;
    KService::List services;

    if (!rulesConfig) {
        return services;
    }

    const int firstSpace = cmdLine.indexOf(' ');
    int slash = 0;

    services = KServiceTypeTrader::self()->query(QStringLiteral("Application"), QStringLiteral("exist Exec and ('%1' =~ Exec)").arg(cmdLine));

    if (services.isEmpty()) {
        // Could not find with complete command line, so strip out the path part ...
        slash = cmdLine.lastIndexOf('/', firstSpace);

        if (slash > 0) {
            services =
                KServiceTypeTrader::self()->query(QStringLiteral("Application"), QStringLiteral("exist Exec and ('%1' =~ Exec)").arg(cmdLine.mid(slash + 1)));
        }
    }

    if (services.isEmpty() && firstSpace > 0) {
        // Could not find with arguments, so try without ...
        cmdLine.truncate(firstSpace);

        services = KServiceTypeTrader::self()->query(QStringLiteral("Application"), QStringLiteral("exist Exec and ('%1' =~ Exec)").arg(cmdLine));

        if (services.isEmpty()) {
            slash = cmdLine.lastIndexOf('/');

            if (slash > 0) {
                services = KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                             QStringLiteral("exist Exec and ('%1' =~ Exec)").arg(cmdLine.mid(slash + 1)));
            }
        }
    }

    if (services.isEmpty()) {
        KConfigGroup set(rulesConfig, "Settings");
        const QStringList &runtimes = set.readEntry("TryIgnoreRuntimes", QStringList());

        bool ignore = runtimes.contains(cmdLine);

        if (!ignore && slash > 0) {
            ignore = runtimes.contains(cmdLine.mid(slash + 1));
        }

        if (ignore) {
            return servicesFromCmdLine(_cmdLine.mid(firstSpace + 1), processName, rulesConfig);
        }
    }

    if (services.isEmpty() && !processName.isEmpty() && !QStandardPaths::findExecutable(cmdLine).isEmpty()) {
        // cmdLine now exists without arguments if there were any.
        services << QExplicitlySharedDataPointer<KService>(new KService(processName, cmdLine, QString()));
    }

    return services;
}

QString defaultApplication(const QUrl &url)
{
    if (url.scheme() != QLatin1String("preferred")) {
        return QString();
    }

    const QString &application = url.host();

    if (application.isEmpty()) {
        return QString();
    }

    if (application.compare(QLatin1String("mailer"), Qt::CaseInsensitive) == 0) {
        KEMailSettings settings;

        // In KToolInvocation, the default is kmail; but let's be friendlier.
        QString command = settings.getSetting(KEMailSettings::ClientProgram);

        if (command.isEmpty()) {
            if (KService::Ptr kontact = KService::serviceByStorageId(QStringLiteral("kontact"))) {
                return kontact->storageId();
            } else if (KService::Ptr kmail = KService::serviceByStorageId(QStringLiteral("kmail"))) {
                return kmail->storageId();
            }
        }

        if (!command.isEmpty()) {
            if (settings.getSetting(KEMailSettings::ClientTerminal) == QLatin1String("true")) {
                KConfigGroup confGroup(KSharedConfig::openConfig(), "General");
                const QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", QStringLiteral("konsole"));
                command = preferredTerminal + QLatin1String(" -e ") + command;
            }

            return command;
        }
    } else if (application.compare(QLatin1String("browser"), Qt::CaseInsensitive) == 0) {
        KConfigGroup config(KSharedConfig::openConfig(), "General");
        QString browserApp = config.readPathEntry("BrowserApplication", QString());

        if (browserApp.isEmpty()) {
            const KService::Ptr htmlApp = KApplicationTrader::preferredService(QStringLiteral("text/html"));

            if (htmlApp) {
                browserApp = htmlApp->storageId();
            }
        } else if (browserApp.startsWith('!')) {
            browserApp.remove(0, 1);
        }

        return browserApp;
    } else if (application.compare(QLatin1String("terminal"), Qt::CaseInsensitive) == 0) {
        KConfigGroup confGroup(KSharedConfig::openConfig(), "General");

        return confGroup.readPathEntry("TerminalApplication", QStringLiteral("konsole"));
    } else if (application.compare(QLatin1String("filemanager"), Qt::CaseInsensitive) == 0) {
        KService::Ptr service = KApplicationTrader::preferredService(QStringLiteral("inode/directory"));

        if (service) {
            return service->storageId();
        }
    } else if (KService::Ptr service = KApplicationTrader::preferredService(application)) {
        return service->storageId();
    }

    return QLatin1String("");
}



}
}
