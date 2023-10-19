// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QCommandLineParser>
#include <QFontDatabase>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtWebView>
#include <clocale>

#ifdef Q_OS_ANDROID
#include "utils/androidutils.h"
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#ifdef HAVE_KDBUSADDONS
#include <KDBusService>
#include <KWindowSystem>
#endif
#include <KConfig>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KWindowConfig>

#include "tokodon-version.h"

#include "account/accountmanager.h"
#include "accountconfig.h"
#include "config.h"
#include "network/networkaccessmanagerfactory.h"
#include "network/networkcontroller.h"
#include "utils/blurhashimageprovider.h"
#include "utils/colorschemer.h"
#include "utils/navigation.h"
#include "utils/windowcontroller.h"

#ifdef Q_OS_WINDOWS
#include <Windows.h>
#endif

#ifdef TEST_MODE
#include "autotests/helperreply.h"
#include "autotests/mockaccount.h"
#endif

using namespace Qt::Literals::StringLiterals;

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char *argv[])
{
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    QtWebView::initialize();

#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    app.connect(&app, &QGuiApplication::applicationStateChanged, [](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive) {
            AndroidUtils::instance().checkPendingIntents();
        }
    });
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QApplication app(argc, argv);
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

#ifdef Q_OS_WINDOWS
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }

    QApplication::setStyle(QStringLiteral("breeze"));
    auto font = app.font();
    font.setPointSize(10);
    app.setFont(font);
#endif
    KLocalizedString::setApplicationDomain("tokodon");

    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));

    KAboutData about(QStringLiteral("tokodon"),
                     i18n("Tokodon"),
                     QStringLiteral(TOKODON_VERSION_STRING),
                     i18n("Mastodon client"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2021-2023 Carl Schwan, 2021-2023 KDE Community"));
    about.addAuthor(i18n("Carl Schwan"),
                    i18n("Maintainer"),
                    QStringLiteral("carl@carlschwan.eu"),
                    QStringLiteral("https://carlschwan.eu"),
                    QStringLiteral("https://carlschwan.eu/avatar.png"));
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QStringLiteral("https://redstrate.com/rss-image.png"));
    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"), i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    about.setOrganizationDomain("kde.org");
    about.setBugAddress("https://bugs.kde.org/describecomponents.cgi?product=tokodon");

    KAboutData::setApplicationData(about);
    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.tokodon")));

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Client for the decentralized social network: mastodon"));
    parser.addPositionalArgument(QStringLiteral("urls"), i18n("Supports web+ap: url scheme"));

    QCommandLineOption shareOption(QStringLiteral("share"), i18n("Share a line of text in the standalone composer."), i18n("The text to share."));
    parser.addOption(shareOption);

    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    setlocale(LC_NUMERIC, "C");

    auto &colorSchemer = ColorSchemer::instance();
    auto config = Config::self();
    if (!config->colorScheme().isEmpty()) {
        colorSchemer.apply(config->colorScheme());
    }

    qmlRegisterSingletonInstance("org.kde.tokodon.private", 1, 0, "Config", config);
    qmlRegisterUncreatableType<AccountConfig>("org.kde.tokodon.private", 1, 0, "AccountConfig", QStringLiteral("Use via Account.config"));

    QQmlApplicationEngine engine;
#ifdef HAVE_KDBUSADDONS
    KDBusService service(KDBusService::Unique);
    QObject::connect(&service, &KDBusService::activateRequested, &engine, [&engine](const QStringList &arguments, const QString & /*workingDirectory*/) {
        const auto rootObjects = engine.rootObjects();
        for (auto obj : rootObjects) {
            if (auto view = qobject_cast<QQuickWindow *>(obj)) {
                KWindowSystem::updateStartupId(view);
                KWindowSystem::activateWindow(view);

                if (arguments.isEmpty()) {
                    return;
                }

                auto args = arguments;
                args.removeFirst();

                if (arguments.length() >= 1) {
                    if (args[0].startsWith("web+ap"_L1)) {
                        NetworkController::instance().openWebApLink(args[0]);
                    } else {
                        NetworkController::instance().setAuthCode(QUrl(args[0]));
                    }
                    if (args[0] == "--share"_L1) {
                        NetworkController::instance().startComposing(args[1]);
                    } else {
                        NetworkController::instance().openWebApLink(args[0]);
                    }
                }

                return;
            }
        }
    });
#endif
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    NetworkAccessManagerFactory namFactory;
    engine.setNetworkAccessManagerFactory(&namFactory);

    engine.addImageProvider(QLatin1String("blurhash"), new BlurhashImageProvider);

#ifdef TEST_MODE
    AccountManager::instance().setTestMode(true);

    auto account = new MockAccount();
    AccountManager::instance().addAccount(account, true);
    AccountManager::instance().selectAccount(account);

    QUrl url = account->apiUrl(QStringLiteral("/api/v2/search"));
    url.setQuery(QUrlQuery{{QStringLiteral("q"), QStringLiteral("myquery")}});
    account->registerGet(url, new TestReply(QStringLiteral("search-result.json"), account));

    account->registerGet(account->apiUrl(QStringLiteral("/api/v1/timelines/home")), new TestReply(QStringLiteral("statuses.json"), account));
#else
    AccountManager::instance().migrateSettings();
    AccountManager::instance().loadFromSettings();
#endif

    if (parser.isSet(shareOption)) {
        engine.loadFromModule("org.kde.tokodon", "StandaloneComposer");

        NetworkController::instance().startComposing(parser.value(shareOption));
    } else {
        engine.loadFromModule("org.kde.tokodon", "Main");

        if (parser.positionalArguments().length() > 0) {
            NetworkController::instance().openWebApLink(parser.positionalArguments()[0]);
        }
    }

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
#ifdef HAVE_KDBUSADDONS
    QQuickWindow *window = nullptr;

    const auto rootObjects = engine.rootObjects();
    for (auto obj : rootObjects) {
        auto view = qobject_cast<QQuickWindow *>(obj);
        if (view) {
            window = view;
            break;
        }
    }

    if (window != nullptr) {
        auto controller = engine.singletonInstance<WindowController *>(QStringLiteral("org.kde.tokodon"), QStringLiteral("WindowController"));
        controller->setWindow(window);
        controller->restoreGeometry();
    }
#endif
    return QCoreApplication::exec();
}
