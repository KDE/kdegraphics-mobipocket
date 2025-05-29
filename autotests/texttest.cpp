// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-Licence-Identifier: BSD-2-Clauses

#include "../lib/mobipocket.h"
#include <QFile>
#include <QObject>
#include <QTest>

class TextTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void benchmarkRead()
    {
        QBENCHMARK {
            QFile file(QStringLiteral(DATA_DIR "/first-men-in-the-moon.azw3"));
            QVERIFY(file.open(QIODevice::ReadOnly));

            Mobipocket::Document document(&file);
            const auto text = document.text();
            QCOMPARE(text.length(), 510364);
        }
    }
};

QTEST_GUILESS_MAIN(TextTest)

#include "texttest.moc"