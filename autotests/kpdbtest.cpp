// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-Licence-Identifier: LGPL-2.1-or-later

#include "../lib/kpdb_p.h"
#include <QFile>
#include <QObject>
#include <QTest>
#include <QTimeZone>

class KPDBTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPDBheaderKf8()
    {
        QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.azw3"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const KPDBFile pdb(file);
        QVERIFY(pdb.isValid());

        const auto header = pdb.header();
        QCOMPARE(header.name().size(), 32);
        QCOMPARE(header.name().replace('\0', ""), "The_First_Men_in_the_Moon");
        QCOMPARE(header.attributes(), 0);
        QCOMPARE(header.attributes(), 0);
        QCOMPARE(header.version(), 0);
        QCOMPARE(header.creationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 25), QTimeZone::utc()));
        QCOMPARE(header.modificationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 25), QTimeZone::utc()));
        QCOMPARE(header.backupTime(), QDateTime::fromSecsSinceEpoch(0, QTimeZone::utc()));
        QCOMPARE(header.modificationNumber(), 0);
        QCOMPARE(header.appInfoOffset(), 0);
        QCOMPARE(header.sortInfoOffset(), 0);
        QCOMPARE(header.databaseType(), "BOOK");
        QCOMPARE(header.creator(), "MOBI");
        QCOMPARE(header.uid(), 291);
        QCOMPARE(header.nextRecord(), 0);
        QCOMPARE(header.recordCount(), 146);

    }

    void testPDBheaderMobi()
    {
        QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.mobi"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const KPDBFile pdb(file);
        QVERIFY(pdb.isValid());

        const auto header = pdb.header();
        QCOMPARE(header.name().size(), 32);
        QCOMPARE(header.name().replace('\0', ""), "The_First_Men_in_the_Moon");
        QCOMPARE(header.attributes(), 0);
        QCOMPARE(header.attributes(), 0);
        QCOMPARE(header.version(), 0);
        QCOMPARE(header.creationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 16), QTimeZone::utc()));
        QCOMPARE(header.modificationTime(), QDateTime(QDate(2025, 05, 01), QTime(23, 10, 16), QTimeZone::utc()));
        QCOMPARE(header.backupTime(), QDateTime::fromSecsSinceEpoch(0, QTimeZone::utc()));
        QCOMPARE(header.modificationNumber(), 0);
        QCOMPARE(header.appInfoOffset(), 0);
        QCOMPARE(header.sortInfoOffset(), 0);
        QCOMPARE(header.databaseType(), "BOOK");
        QCOMPARE(header.creator(), "MOBI");
        QCOMPARE(header.uid(), 245);
        QCOMPARE(header.nextRecord(), 0);
        QCOMPARE(header.recordCount(), 123);
    }
};

QTEST_MAIN(KPDBTest)
#include "kpdbtest.moc"
