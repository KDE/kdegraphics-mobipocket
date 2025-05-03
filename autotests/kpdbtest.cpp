// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-Licence-Identifier: BSD-2-Clauses

#include "../lib/kpdb.h"
#include <QFile>
#include <QObject>
#include <QTest>

class KPDBTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPDBheaderKf8()
    {
        KPDBFile pdb;
        {
            QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.azw3"));
            QVERIFY(file.open(QIODevice::ReadOnly));
            pdb.read(&file);
        }

        QCOMPARE(pdb.header().name().size(), 32);
        QCOMPARE(pdb.header().name().replace('\0', ""), "The_First_Men_in_the_Moon");
        QCOMPARE(pdb.header().attributes(), 0);
        QCOMPARE(pdb.header().attributes(), 0);
        QCOMPARE(pdb.header().version(), 0);
        QCOMPARE(pdb.header().creationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 25), QTimeZone::UTC));
        QCOMPARE(pdb.header().modificationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 25), QTimeZone::UTC));
        QCOMPARE(pdb.header().backupTime(), QDateTime::fromSecsSinceEpoch(0, QTimeZone::UTC));
        QCOMPARE(pdb.header().modificationNumber(), 0);
        QCOMPARE(pdb.header().appInfoOffset(), 0);
        QCOMPARE(pdb.header().sortInfoOffset(), 0);
        QCOMPARE(pdb.header().databaseType(), "BOOK");
        QCOMPARE(pdb.header().creator(), "MOBI");
        QCOMPARE(pdb.header().uid(), 291);
        QCOMPARE(pdb.header().nextRecord(), 0);
        QCOMPARE(pdb.header().recordCount(), 146);

        QVERIFY(pdb.isValid());
    }

    void testPDBheaderMobi()
    {
        KPDBFile pdb;
        {
            QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.mobi"));
            QVERIFY(file.open(QIODevice::ReadOnly));
            pdb.read(&file);
        }

        QCOMPARE(pdb.header().name().size(), 32);
        QCOMPARE(pdb.header().name().replace('\0', ""), "The_First_Men_in_the_Moon");
        QCOMPARE(pdb.header().attributes(), 0);
        QCOMPARE(pdb.header().attributes(), 0);
        QCOMPARE(pdb.header().version(), 0);
        QCOMPARE(pdb.header().creationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 16), QTimeZone::UTC));
        QCOMPARE(pdb.header().modificationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 16), QTimeZone::UTC));
        QCOMPARE(pdb.header().backupTime(), QDateTime::fromSecsSinceEpoch(0, QTimeZone::UTC));
        QCOMPARE(pdb.header().modificationNumber(), 0);
        QCOMPARE(pdb.header().appInfoOffset(), 0);
        QCOMPARE(pdb.header().sortInfoOffset(), 0);
        QCOMPARE(pdb.header().databaseType(), "BOOK");
        QCOMPARE(pdb.header().creator(), "MOBI");
        QCOMPARE(pdb.header().uid(), 245);
        QCOMPARE(pdb.header().nextRecord(), 0);
        QCOMPARE(pdb.header().recordCount(), 123);

        QVERIFY(pdb.isValid());
    }
};

QTEST_MAIN(KPDBTest)
#include "kpdbtest.moc"
