/**
 * @file
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "json.h"
#include <QVariant>
#include <QList>
#include <QSet>
#include <QDebug>

#if QT_VERSION > 0x050000
#include <QJsonObject>
#include <QJsonParseError>
#else
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif


using namespace Example;

QByteArray JSON::json(const QVariant& data) {
    #if QT_VERSION > 0x050000
    QByteArray json = QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact);
    return json;

    #else

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
    #endif
}


QVariant JSON::data(const QByteArray& json) {
    #if QT_VERSION > 0x050000
    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(json, &error);
    return jdoc.toVariant();
    #else
    bool ok;
    QJson::Parser parser;
    return parser.parse(json, &ok);
    #endif
}

