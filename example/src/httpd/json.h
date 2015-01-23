#ifndef _JSON_H_
#define _JSON_H_

#include <QVariantMap>
#include <QByteArray>
#include <QVariant>
#include <QList>
#include <QSet>

class JSON {
  public:
    static QByteArray json(const QVariant&);
//  static QByteArray json(const QSet<QString>&);
//  static QByteArray json(const QList<QString>&);

    static QVariantMap data(const QByteArray& json);
};

#endif
