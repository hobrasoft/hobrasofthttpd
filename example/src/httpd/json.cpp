#include "json.h"
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <QVariant>
#include <QList>
#include <QSet>
#include <QDebug>


QByteArray JSON::json(const QVariant& data) {
    bool ok = true;
    QJson::Serializer serializer;


#ifdef ARM
    QByteArray json = serializer.serialize(data);
#else
    QByteArray json = serializer.serialize(data, &ok);
#endif
    if (!ok) {
        qDebug() << QString("Sorry, there is a problem with data serializing in JSON::json()");
        }

    return json;
}


QVariantMap JSON::data(const QByteArray& json) {
    bool ok;
    QJson::Parser parser;
    return parser.parse(json, &ok).toMap();
}

