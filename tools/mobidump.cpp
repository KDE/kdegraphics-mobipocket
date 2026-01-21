#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTextStream>

#include "mobipocket.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOption({{QStringLiteral("f"), QStringLiteral("fulltext")}, QStringLiteral("Show full text")});
    parser.addPositionalArgument(QStringLiteral("filename"), QStringLiteral("File to process"));
    parser.process(app);

    if (parser.positionalArguments().size() != 1) {
        QTextStream(stderr) << "Exactly one argument is accepted" << Qt::endl;
        parser.showHelp(1);
    }
    bool showFulltext = parser.isSet(QStringLiteral("fulltext"));

    auto fi = QFileInfo(parser.positionalArguments().at(0));
    QString url = fi.absoluteFilePath();

    if (!fi.exists()) {
        QTextStream(stderr) << "File " << url << " not found" << Qt::endl;
        return 1;
    }

    if (!fi.isFile() || !fi.isReadable()) {
        QTextStream(stderr) << "File " << url << " is not a readable file" << Qt::endl;
        return 1;
    }

    QFile file(url);
    if (!file.open(QFile::ReadOnly)) {
        QTextStream(stderr) << "Failed to open " << url << Qt::endl;
    }
    Mobipocket::Document doc(&file);

    if (!doc.isValid()) {
        QTextStream(stderr) << "File " << url << " is not a valid MobiPocket file" << Qt::endl;
        return 1;
    }

    QTextStream out(stdout);
    out << "===\nFile metadata:" << Qt::endl;
    for (const auto &meta : doc.metadata().asKeyValueRange()) {
        out << meta.first << " \"" << meta.second << "\"" << Qt::endl;
    }
    out << "DRM protected:" << (doc.hasDRM() ? " yes" : " no") << Qt::endl;
    if (showFulltext && !doc.hasDRM()) {
        out << "===\nRaw text:" << Qt::endl;
        out << "\"" << doc.text() << "\"" << Qt::endl;
    }
    out << "===" << Qt::endl << Qt::endl;
    return 0;
}
