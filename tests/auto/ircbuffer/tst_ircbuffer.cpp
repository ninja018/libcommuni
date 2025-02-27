/*
 * Copyright (C) 2008-2020 The Communi Project
 *
 * This test is free, and not covered by the BSD license. There is no
 * restriction applied to their modification, redistribution, using and so on.
 * You can study them, modify them, use them in your own program - either
 * completely or partially.
 */

#include "ircbuffer.h"
#include "ircbuffermodel.h"
#include "ircconnection.h"
#include "irccommand.h"
#include "ircmessage.h"
#include "ircfilter.h"
#include <QtTest/QtTest>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #include <QtCore/QRegExp>
#else
    #include <QRegularExpression>
#endif

class tst_IrcBuffer : public QObject
{
    Q_OBJECT

private slots:
    void testDefaults();
    void testTitleNamePrefix();
    void testSticky();
    void testPersistent();
    void testReceive();
    void testDebug();
    void testUserData();
    void testClose();
    void testSendCommand();
};

void tst_IrcBuffer::testDefaults()
{
    IrcBuffer buffer;
    QVERIFY(buffer.title().isEmpty());
    QVERIFY(buffer.name().isEmpty());
    QVERIFY(buffer.prefix().isEmpty());
    QVERIFY(!buffer.isChannel());
    QVERIFY(!buffer.toChannel());
    QVERIFY(!buffer.connection());
    QVERIFY(!buffer.network());
    QVERIFY(!buffer.model());
    QVERIFY(!buffer.isActive());
    QVERIFY(!buffer.isSticky());
    QVERIFY(!buffer.isPersistent());
    QVERIFY(buffer.userData().isEmpty());
}

void tst_IrcBuffer::testTitleNamePrefix()
{
    IrcBuffer buffer;

    QSignalSpy titleSpy(&buffer, SIGNAL(titleChanged(QString)));
    QSignalSpy nameSpy(&buffer, SIGNAL(nameChanged(QString)));
    QSignalSpy prefixSpy(&buffer, SIGNAL(prefixChanged(QString)));
    QVERIFY(titleSpy.isValid());
    QVERIFY(nameSpy.isValid());
    QVERIFY(prefixSpy.isValid());

    buffer.setName("name");
    QCOMPARE(buffer.title(), QString("name"));
    QCOMPARE(buffer.name(), QString("name"));
    QCOMPARE(buffer.prefix(), QString());
    QCOMPARE(titleSpy.count(), 1);
    QCOMPARE(titleSpy.last().first().toString(), QString("name"));
    QCOMPARE(nameSpy.count(), 1);
    QCOMPARE(nameSpy.last().first().toString(), QString("name"));
    QCOMPARE(prefixSpy.count(), 0);

    buffer.setPrefix("prefix");
    QCOMPARE(buffer.title(), QString("prefixname"));
    QCOMPARE(buffer.name(), QString("name"));
    QCOMPARE(buffer.prefix(), QString("prefix"));
    QCOMPARE(titleSpy.count(), 2);
    QCOMPARE(titleSpy.last().first().toString(), QString("prefixname"));
    QCOMPARE(nameSpy.count(), 1);
    QCOMPARE(prefixSpy.count(), 1);
    QCOMPARE(prefixSpy.last().first().toString(), QString("prefix"));
}

void tst_IrcBuffer::testSticky()
{
    IrcBuffer buffer;
    QVERIFY(!buffer.isSticky());

    QSignalSpy spy(&buffer, SIGNAL(stickyChanged(bool)));
    QVERIFY(spy.isValid());

    buffer.setSticky(true);
    QVERIFY(buffer.isSticky());
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.last().last().toBool());

    buffer.setSticky(false);
    QVERIFY(!buffer.isSticky());
    QCOMPARE(spy.count(), 2);
    QVERIFY(!spy.last().last().toBool());
}

void tst_IrcBuffer::testPersistent()
{
    IrcBuffer buffer;
    QVERIFY(!buffer.isPersistent());

    QSignalSpy spy(&buffer, SIGNAL(persistentChanged(bool)));
    QVERIFY(spy.isValid());

    buffer.setPersistent(true);
    QVERIFY(buffer.isPersistent());
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.last().last().toBool());

    buffer.setPersistent(false);
    QVERIFY(!buffer.isPersistent());
    QCOMPARE(spy.count(), 2);
    QVERIFY(!spy.last().last().toBool());
}

void tst_IrcBuffer::testReceive()
{
    Irc::registerMetaTypes();

    IrcBuffer buffer;

    QSignalSpy spy(&buffer, SIGNAL(messageReceived(IrcMessage*)));
    QVERIFY(spy.isValid());

    buffer.receiveMessage(nullptr);
    QCOMPARE(spy.count(), 0);

    IrcMessage msg(nullptr);
    buffer.receiveMessage(&msg);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0).value<IrcMessage*>(), &msg);
}

void tst_IrcBuffer::testDebug()
{
    QString str;
    QDebug dbg(&str);

    dbg << static_cast<IrcBuffer*>(nullptr);
    QCOMPARE(str.trimmed(), QString::fromLatin1("IrcBuffer(0x0)"));
    str.clear();

    IrcBuffer buffer;
    dbg << &buffer;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QRegExp("IrcBuffer\\(0x[0-9A-Fa-f]+\\) ").exactMatch(str));
#else
    QVERIFY(QRegularExpression("IrcBuffer\\(0x[0-9A-Fa-f]+\\) ").match(str).hasMatch());
#endif
    str.clear();

    buffer.setObjectName("obj");
    dbg << &buffer;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QRegExp("IrcBuffer\\(0x[0-9A-Fa-f]+, name=obj\\) ").exactMatch(str));
#else
    QVERIFY(QRegularExpression("IrcBuffer\\(0x[0-9A-Fa-f]+, name=obj\\) ").match(str).hasMatch());
#endif
    str.clear();

    buffer.setName("buf");
    dbg << &buffer;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QRegExp("IrcBuffer\\(0x[0-9A-Fa-f]+, name=obj, title=buf\\) ").exactMatch(str));
#else
    QVERIFY(QRegularExpression("IrcBuffer\\(0x[0-9A-Fa-f]+, name=obj, title=buf\\) ").match(str).hasMatch());
#endif
    str.clear();
}

void tst_IrcBuffer::testUserData()
{
    QVariantMap ud;
    ud.insert("foo", "bar");

    IrcBuffer buffer;
    buffer.setUserData(ud);
    QCOMPARE(buffer.userData(), ud);

    buffer.setUserData(QVariantMap());
    QVERIFY(buffer.userData().isEmpty());
}

void tst_IrcBuffer::testClose()
{
    IrcBufferModel model;
    QPointer<IrcBuffer> buffer = model.add("foo");
    buffer->close();
    QVERIFY(!model.contains("foo"));
    QVERIFY(!buffer);
}

class TestCommandFilter : public QObject, public IrcCommandFilter
{
    Q_OBJECT
    Q_INTERFACES(IrcCommandFilter)

public:
    TestCommandFilter(IrcConnection* connection) : lastCommand(nullptr) { connection->installCommandFilter(this); }
    bool commandFilter(IrcCommand *command) override { lastCommand = command; return true; }
    IrcCommand* lastCommand;
};

void tst_IrcBuffer::testSendCommand()
{
    IrcConnection connection;
    TestCommandFilter filter(&connection);

    IrcBufferModel model(&connection);
    QCOMPARE(model.connection(), &connection);

    IrcBuffer* buffer = model.add("foo");
    QCOMPARE(buffer->connection(), &connection);

    IrcCommand* cmd = IrcCommand::createAway();
    QVERIFY(!buffer->sendCommand(cmd));
    QCOMPARE(filter.lastCommand, cmd);
}

QTEST_MAIN(tst_IrcBuffer)

#include "tst_ircbuffer.moc"
