/***************************************************************************
 *   Copyright (C) 2008 by Jakub Stachowski <qbast@go2.pl>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef MOBI_DOCUMENT_H
#define MOBI_DOCUMENT_H

#include <QTextDocument>
#include <QUrl>
#include <QVariant>

class QFile;
namespace Mobipocket {
class Document;
class QFileStream;
}

namespace Mobi {

  class MobiDocument : public QTextDocument {
      
  public:
    MobiDocument(const QString &fileName);  
    ~MobiDocument();   
    
    Mobipocket::Document* mobi() const { return doc; }
    
  protected:
    virtual QVariant loadResource(int type, const QUrl &name);
    
  private:
    QString fixMobiMarkup(const QString& data);
    Mobipocket::Document *doc;
    Mobipocket::QFileStream* file;
  };

}
#endif
