/*
 * Copyright (C) 2008-2020 The Communi Project
 *
 * This test is free, and not covered by the BSD license. There is no
 * restriction applied to their modification, redistribution, using and so on.
 * You can study them, modify them, use them in your own program - either
 * completely or partially.
 */

#include "irccommand.h"
#include "ircmessage.h"
#include "ircconnection.h"
#include <QtTest/QtTest>
#include <QtCore/QScopedPointer>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QtCore/QRegExp>
    #include <QtCore/QTextCodec>
#else
    #include <QRegularExpression>
#endif

class tst_IrcCommand : public QObject
{
    Q_OBJECT

private slots:
    void testDefaults();

    void testEncoding_data();
    void testEncoding();

    void testConversion();

    void testConnection();

    void testAdmin();
    void testAway();
    void testCapability();
    void testCtcpAction();
    void testCtcpReply();
    void testCtcpRequest();
    void testInfo();
    void testInvite();
    void testJoin();
    void testKick();
    void testKnock();
    void testList();
    void testMessage();
    void testMode();
    void testMonitor();
    void testMotd();
    void testNames();
    void testNick();
    void testNotice();
    void testPart();
    void testPing();
    void testPong();
    void testQuit();
    void testQuote();
    void testStats();
    void testTime();
    void testTopic();
    void testTrace();
    void testUsers();
    void testVersion();
    void testWho();
    void testWhois();
    void testWhowas();

    void testDebug();

private:
    void verifyIfTextContainsPattern(const QString& text, const QString& pattern);
};

void tst_IrcCommand::testDefaults()
{
    IrcCommand cmd;
    QVERIFY(cmd.parameters().isEmpty());
    QCOMPARE(cmd.type(), IrcCommand::Custom);
    QCOMPARE(cmd.encoding(), QByteArray("UTF-8"));
    QVERIFY(!cmd.connection());
    QVERIFY(!cmd.network());

    QTest::ignoreMessage(QtWarningMsg, "Reimplement IrcCommand::toString() for IrcCommand::Custom");
    QVERIFY(cmd.toString().isEmpty());
}

void tst_IrcCommand::testEncoding_data()
{
    QTest::addColumn<QByteArray>("encoding");
    QTest::addColumn<QByteArray>("actual");
    QTest::addColumn<bool>("supported");

    QTest::newRow("null") << QByteArray() << QByteArray("UTF-8") << false;
    QTest::newRow("empty") << QByteArray("") << QByteArray("UTF-8") << false;
    QTest::newRow("space") << QByteArray(" ") << QByteArray("UTF-8") << false;
    QTest::newRow("invalid") << QByteArray("invalid") << QByteArray("UTF-8") << false;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    foreach (const QByteArray& codec, QTextCodec::availableCodecs())
        QTest::newRow(codec) << codec << codec << true;
#endif
}

void tst_IrcCommand::testEncoding()
{
    QFETCH(QByteArray, encoding);
    QFETCH(QByteArray, actual);
    QFETCH(bool, supported);

    if (!supported)
        QTest::ignoreMessage(QtWarningMsg, "IrcCommand::setEncoding(): unsupported encoding \"" + encoding + "\" ");

    IrcCommand cmd;
    cmd.setEncoding(encoding);
    QCOMPARE(cmd.encoding(), actual);
}

void tst_IrcCommand::testConversion()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createMessage("target", "foo bar"));
    QVERIFY(cmd.data());
    QCOMPARE(cmd->type(), IrcCommand::Message);

    IrcConnection conn;
    QScopedPointer<IrcMessage> msg(cmd->toMessage("prefix", &conn));
    QVERIFY(msg.data());

    QCOMPARE(msg->type(), IrcMessage::Private);
    QCOMPARE(msg->connection(), &conn);
    QCOMPARE(msg->prefix(), QString("prefix"));
    QCOMPARE(msg->property("target").toString(), QString("target"));
    QCOMPARE(msg->property("content").toString(), QString("foo bar"));
}

void tst_IrcCommand::testConnection()
{
    IrcConnection* connection = new IrcConnection(this);
    IrcCommand command(connection);
    QVERIFY(!command.connection());
    QVERIFY(!command.network());

    connection->sendCommand(&command);
    QCOMPARE(command.connection(), connection);
    QCOMPARE(command.network(), connection->network());

    command.setParent(nullptr);
    delete connection;
    QVERIFY(!command.connection());
    QVERIFY(!command.network());
}

void tst_IrcCommand::testAdmin()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createAdmin("server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Admin);
    verifyIfTextContainsPattern(cmd->toString(), "\\bADMIN\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testAway()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createAway("reason"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Away);
    verifyIfTextContainsPattern(cmd->toString(), "\\bAWAY\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\breason\\b");
}

void tst_IrcCommand::testCapability()
{
    QScopedPointer<IrcCommand> cmd1(IrcCommand::createCapability("sub", QString("cap")));
    QVERIFY(cmd1.data());

    QCOMPARE(cmd1->type(), IrcCommand::Capability);
    verifyIfTextContainsPattern(cmd1->toString(), "\\bCAP\\b");
    verifyIfTextContainsPattern(cmd1->toString(), "\\bsub\\b");
    verifyIfTextContainsPattern(cmd1->toString(), "\\bcap\\b");

    QScopedPointer<IrcCommand> cmd2(IrcCommand::createCapability("sub", QStringList() << "cap1" << "cap2"));
    QVERIFY(cmd2.data());

    QCOMPARE(cmd2->type(), IrcCommand::Capability);
    verifyIfTextContainsPattern(cmd2->toString(), "\\bCAP\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bsub\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bcap1\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bcap2\\b");
}

void tst_IrcCommand::testCtcpAction()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createCtcpAction("tgt", "act"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::CtcpAction);
    verifyIfTextContainsPattern(cmd->toString(), "\\bPRIVMSG\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btgt\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bact\\b");
    QCOMPARE(cmd->toString().count("\01"), 2);
}

void tst_IrcCommand::testCtcpReply()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createCtcpReply("tgt", "rpl"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::CtcpReply);
    verifyIfTextContainsPattern(cmd->toString(), "\\bNOTICE\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btgt\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\brpl\\b");
    QCOMPARE(cmd->toString().count("\01"), 2);
}

void tst_IrcCommand::testCtcpRequest()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createCtcpRequest("tgt", "req"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::CtcpRequest);
    verifyIfTextContainsPattern(cmd->toString(), "\\bPRIVMSG\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btgt\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\breq\\b");
    QCOMPARE(cmd->toString().count("\01"), 2);
}

void tst_IrcCommand::testInfo()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createInfo("server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Info);
    verifyIfTextContainsPattern(cmd->toString(), "\\bINFO\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testInvite()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createInvite("usr", "chan"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Invite);
    verifyIfTextContainsPattern(cmd->toString(), "\\bINVITE\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\busr\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bchan\\b");
}

void tst_IrcCommand::testJoin()
{
    QScopedPointer<IrcCommand> cmd1(IrcCommand::createJoin("chan"));
    QVERIFY(cmd1.data());

    QCOMPARE(cmd1->type(), IrcCommand::Join);
    verifyIfTextContainsPattern(cmd1->toString(), "\\bJOIN\\b");
    verifyIfTextContainsPattern(cmd1->toString(), "\\bchan\\b");

    QScopedPointer<IrcCommand> cmd2(IrcCommand::createJoin(QStringList() << "chan1" << "chan2"));
    QVERIFY(cmd2.data());

    QCOMPARE(cmd2->type(), IrcCommand::Join);
    verifyIfTextContainsPattern(cmd2->toString(), "\\bJOIN\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bchan1\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bchan2\\b");

    QScopedPointer<IrcCommand> cmd3(IrcCommand::createJoin(QStringList() << "chan1" << "chan2", QStringList() << "key1" << "key2"));
    QVERIFY(cmd3.data());

    QCOMPARE(cmd3->type(), IrcCommand::Join);
    verifyIfTextContainsPattern(cmd3->toString(), "\\bJOIN\\b");
    verifyIfTextContainsPattern(cmd3->toString(), "\\bchan1,chan2\\b");
    verifyIfTextContainsPattern(cmd3->toString(), "\\bkey1,key2\\b");
}

void tst_IrcCommand::testKick()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createKick("chan", "usr"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Kick);
    verifyIfTextContainsPattern(cmd->toString(), "\\bKICK\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bchan\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\busr\\b");
}

void tst_IrcCommand::testKnock()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createKnock("chan"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Knock);
    verifyIfTextContainsPattern(cmd->toString(), "\\bKNOCK\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bchan\\b");
}

void tst_IrcCommand::testList()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createList(QStringList() << "chan1" << "chan2", "server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::List);
    verifyIfTextContainsPattern(cmd->toString(), "\\bLIST\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bchan1\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bchan2\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testMessage()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createMessage("tgt", "msg"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Message);
    verifyIfTextContainsPattern(cmd->toString(), "\\bPRIVMSG\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btgt\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bmsg\\b");
}

void tst_IrcCommand::testMode()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createMode("tgt", "mode"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Mode);
    verifyIfTextContainsPattern(cmd->toString(), "\\bMODE\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btgt\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bmode\\b");
}

void tst_IrcCommand::testMonitor()
{
    QScopedPointer<IrcCommand> cmd1(IrcCommand::createMonitor("+", "foo"));
    QVERIFY(cmd1.data());

    QCOMPARE(cmd1->type(), IrcCommand::Monitor);
    verifyIfTextContainsPattern(cmd1->toString(), "\\bMONITOR\\b");
    verifyIfTextContainsPattern(cmd1->toString(), "\\bfoo\\b");

    QScopedPointer<IrcCommand> cmd2(IrcCommand::createMonitor("+", QStringList() << "foo" << "bar"));
    QVERIFY(cmd2.data());

    QCOMPARE(cmd2->type(), IrcCommand::Monitor);
    verifyIfTextContainsPattern(cmd2->toString(), "\\bMONITOR\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bfoo\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bbar\\b");
}

void tst_IrcCommand::testMotd()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createMotd("server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Motd);
    verifyIfTextContainsPattern(cmd->toString(), "\\bMOTD\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testNames()
{
    QScopedPointer<IrcCommand> cmd1(IrcCommand::createNames("chan"));
    QVERIFY(cmd1.data());

    QCOMPARE(cmd1->type(), IrcCommand::Names);
    verifyIfTextContainsPattern(cmd1->toString(), "\\bNAMES\\b");
    verifyIfTextContainsPattern(cmd1->toString(), "\\bchan\\b");

    QScopedPointer<IrcCommand> cmd2(IrcCommand::createNames(QStringList() << "chan1" << "chan2"));
    QVERIFY(cmd2.data());

    QCOMPARE(cmd2->type(), IrcCommand::Names);
    verifyIfTextContainsPattern(cmd2->toString(), "\\bNAMES\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bchan1\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bchan2\\b");
}

void tst_IrcCommand::testNick()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createNick("nick"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Nick);
    verifyIfTextContainsPattern(cmd->toString(), "\\bNICK\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bnick\\b");
}

void tst_IrcCommand::testNotice()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createNotice("tgt", "msg"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Notice);
    verifyIfTextContainsPattern(cmd->toString(), "\\bNOTICE\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btgt\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bmsg\\b");
}

void tst_IrcCommand::testPart()
{
    QScopedPointer<IrcCommand> cmd1(IrcCommand::createPart("chan"));
    QVERIFY(cmd1.data());

    QCOMPARE(cmd1->type(), IrcCommand::Part);
    verifyIfTextContainsPattern(cmd1->toString(), "\\bPART\\b");
    verifyIfTextContainsPattern(cmd1->toString(), "\\bchan\\b");

    QScopedPointer<IrcCommand> cmd2(IrcCommand::createPart(QStringList() << "chan1" << "chan2"));
    QVERIFY(cmd2.data());

    QCOMPARE(cmd2->type(), IrcCommand::Part);
    verifyIfTextContainsPattern(cmd2->toString(), "\\bPART\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bchan1\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bchan2\\b");
}

void tst_IrcCommand::testPing()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createPing("arg"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Ping);
    verifyIfTextContainsPattern(cmd->toString(), "\\bPING\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\barg\\b");
}

void tst_IrcCommand::testPong()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createPong("arg"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Pong);
    verifyIfTextContainsPattern(cmd->toString(), "\\bPONG\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\barg\\b");
}

void tst_IrcCommand::testQuit()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createQuit("reason"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Quit);
    verifyIfTextContainsPattern(cmd->toString(), "\\bQUIT\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\breason\\b");
}

void tst_IrcCommand::testQuote()
{
    QScopedPointer<IrcCommand> cmd1(IrcCommand::createQuote("CUSTOM"));
    QVERIFY(cmd1.data());

    QCOMPARE(cmd1->type(), IrcCommand::Quote);
    verifyIfTextContainsPattern(cmd1->toString(), "\\bCUSTOM\\b");

    QScopedPointer<IrcCommand> cmd2(IrcCommand::createQuote(QStringList() << "FOO" << "BAR"));
    QVERIFY(cmd2.data());

    QCOMPARE(cmd2->type(), IrcCommand::Quote);
    verifyIfTextContainsPattern(cmd2->toString(), "\\bFOO\\b");
    verifyIfTextContainsPattern(cmd2->toString(), "\\bBAR\\b");
}

void tst_IrcCommand::testStats()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createStats("query", "server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Stats);
    verifyIfTextContainsPattern(cmd->toString(), "\\bSTATS\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bquery\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testTime()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createTime("server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Time);
    verifyIfTextContainsPattern(cmd->toString(), "\\bTIME\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testTopic()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createTopic("chan", "topic"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Topic);
    verifyIfTextContainsPattern(cmd->toString(), "\\bTOPIC\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bchan\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btopic\\b");
}

void tst_IrcCommand::testTrace()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createTrace("target"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Trace);
    verifyIfTextContainsPattern(cmd->toString(), "\\bTRACE\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\btarget\\b");
}

void tst_IrcCommand::testUsers()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createUsers("server"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Users);
    verifyIfTextContainsPattern(cmd->toString(), "\\bUSERS\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bserver\\b");
}

void tst_IrcCommand::testVersion()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createVersion("user"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Version);
    verifyIfTextContainsPattern(cmd->toString(), "\\bVERSION\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\buser\\b");
}

void tst_IrcCommand::testWho()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createWho("mask"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Who);
    verifyIfTextContainsPattern(cmd->toString(), "\\bWHO\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bmask\\b");
}

void tst_IrcCommand::testWhois()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createWhois("mask"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Whois);
    verifyIfTextContainsPattern(cmd->toString(), "\\bWHOIS\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bmask\\b");
}

void tst_IrcCommand::testWhowas()
{
    QScopedPointer<IrcCommand> cmd(IrcCommand::createWhowas("mask"));
    QVERIFY(cmd.data());

    QCOMPARE(cmd->type(), IrcCommand::Whowas);
    verifyIfTextContainsPattern(cmd->toString(), "\\bWHOWAS\\b");
    verifyIfTextContainsPattern(cmd->toString(), "\\bmask\\b");
}

void tst_IrcCommand::testDebug()
{
    QString str;
    QDebug dbg(&str);

    dbg << static_cast<IrcCommand*>(nullptr);
    QCOMPARE(str.trimmed(), QString::fromLatin1("IrcCommand(0x0)"));
    str.clear();

    IrcCommand command;
    QTest::ignoreMessage(QtWarningMsg, "Reimplement IrcCommand::toString() for IrcCommand::Custom");
    dbg << &command;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QRegExp("IrcCommand\\(0x[0-9A-Fa-f]+, type=Custom\\) ").exactMatch(str));
#else
    QVERIFY(QRegularExpression("IrcCommand\\(0x[0-9A-Fa-f]+, type=Custom\\) ").match(str).hasMatch());
#endif
    str.clear();

    command.setType(IrcCommand::Quit);
    dbg << &command;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QRegExp("IrcCommand\\(0x[0-9A-Fa-f]+, type=Quit, \"QUIT :\"\\) ").exactMatch(str));
#else
    QVERIFY(QRegularExpression("IrcCommand\\(0x[0-9A-Fa-f]+, type=Quit, \"QUIT :\"\\) ").match(str).hasMatch());
#endif
    str.clear();

    command.setObjectName("foo");
    dbg << &command;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QRegExp("IrcCommand\\(0x[0-9A-Fa-f]+, name=foo, type=Quit, \"QUIT :\"\\) ").exactMatch(str));
#else
    QVERIFY(QRegularExpression("IrcCommand\\(0x[0-9A-Fa-f]+, name=foo, type=Quit, \"QUIT :\"\\) ").match(str).hasMatch());
#endif
    str.clear();

    dbg << IrcCommand::Join;
    QCOMPARE(str.trimmed(), QString::fromLatin1("Join"));
    str.clear();
}

void tst_IrcCommand::verifyIfTextContainsPattern(const QString& text, const QString& pattern)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(text.contains(QRegExp(pattern);
#else
    QVERIFY(text.contains(QRegularExpression(pattern)));
#endif
}

QTEST_MAIN(tst_IrcCommand)

#include "tst_irccommand.moc"
