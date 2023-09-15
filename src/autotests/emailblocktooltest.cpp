// SPDX-FileCopyrightText: 2023 Rishi Kumar <rsi.dev17@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "account/accountmanager.h"
#include "admin/emailblocktoolmodel.h"
#include "helperreply.h"
#include "mockaccount.h"

#include <QAbstractItemModelTester>
#include <QSignalSpy>
#include <QtTest/QtTest>

class IpRulesToolTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        account = new MockAccount();
        AccountManager::instance().addAccount(account, false);
        AccountManager::instance().selectAccount(account);
    }

    void cleanupTestCase()
    {
        AccountManager::instance().removeAccount(account);
    }

    void testModel()
    {
        QUrl url = account->apiUrl("/api/v1/admin/email_domain_blocks");
        account->registerGet(url, new TestReply("email-info.json", account));

        EmailBlockToolModel emailBlockToolModel;
        QCOMPARE(emailBlockToolModel.rowCount({}), 2);
        QCOMPARE(emailBlockToolModel.data(emailBlockToolModel.index(0, 0), EmailBlockToolModel::IdRole).toInt(), 2);
        QCOMPARE(emailBlockToolModel.data(emailBlockToolModel.index(0, 0), EmailBlockToolModel::DomainRole).toString(), "kde.org");
        Q_ASSERT(emailBlockToolModel.data(emailBlockToolModel.index(0, 0), EmailBlockToolModel::CreatedAtRole).isValid());
        QCOMPARE(emailBlockToolModel.data(emailBlockToolModel.index(0, 0), EmailBlockToolModel::AccountSignUpCount).toInt(), 112);
        QCOMPARE(emailBlockToolModel.data(emailBlockToolModel.index(0, 0), EmailBlockToolModel::IpSignUpCount).toInt(), 255);
    }

private:
    MockAccount *account;
};

QTEST_MAIN(IpRulesToolTest)
#include "emailblocktooltest.moc"