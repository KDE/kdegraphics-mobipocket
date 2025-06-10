#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

#include "mobipocket.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOption({{QStringLiteral("f"), QStringLiteral("fulltext")}, QStringLiteral("Show full text")});
    parser.addPositionalArgument(QStringLiteral("filename"), QStringLiteral("File to process"));
    parser.process(app);

    if (parser.positionalArguments().size() != 1) {
        qDebug() << "Exactly one argument is accepted";
        parser.showHelp(1);
    }
    bool showFulltext = parser.isSet(QStringLiteral("fulltext"));

    auto fi = QFileInfo(parser.positionalArguments().at(0));
    QString url = fi.absoluteFilePath();

    if (!fi.exists()) {
        qDebug() << "File" << url << "not found";
        return 1;
    }

    if (!fi.isFile() || !fi.isReadable()) {
        qDebug() << "File" << url << "is not a readable file";
        return 1;
    }

    QFile file(url);
    file.open(QFile::ReadOnly);
    Mobipocket::Document doc(&file);

    if (!doc.isValid()) {
        qDebug() << "File" << url << "is not a valid MobiPocket file";
        return 1;
    }

    qDebug() << "===\nFile metadata:";
    for (const auto &meta : doc.metadata().asKeyValueRange()) {
        qDebug() << meta.first << meta.second;
    }
    qDebug() << "DRM protected:" << (doc.hasDRM() ? "yes" : "no");
    if (showFulltext && !doc.hasDRM()) {
        qDebug() << "===\nRaw text:";
        qDebug() << doc.text();
    }
    qDebug() << "===\n";
}
