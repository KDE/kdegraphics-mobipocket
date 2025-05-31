// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../lib/mobipocket.h"
#include <QCommandLineParser>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QTextDocument>

int main(int argc, char *argv[])
{
    QCommandLineParser parser;
    QGuiApplication app(argc, argv);
    parser.addPositionalArgument(QStringLiteral("source"), {});
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty()) {
        qWarning() << "No file given";
        return 1;
    }

    QFile file(args[0]);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file";
        return 1;
    }

    Mobipocket::Document document(&file);
    const auto metadata = document.metadata();
    for (const auto &[key, metadata] : metadata.asKeyValueRange()) {
        qWarning().noquote() << document.formatMetadata(key, metadata);
    }

    qWarning() << "Is KF8:" << (document.isKF8() ? "yes" : "no");

    QElapsedTimer timer;
    timer.start();
    const auto text = document.plainText();
    qWarning() << "Content size:" << text.length();
    timer.elapsed();
    qWarning() << "Read all the content in" << timer.elapsed() << "milliseconds";

    qWarning() << "Plain text content (truncated):\n" << text.left(3000);

    return 0;
}
