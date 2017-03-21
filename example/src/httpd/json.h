/**
 * @file
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _JSON_H_
#define _JSON_H_

#include <QVariant>
#include <QByteArray>
#include <QVariant>
#include <QList>
#include <QSet>

namespace Example {
class JSON {
  public:
    static QByteArray json(const QVariant& data);      ///< Converts data to json
    static QVariant   data(const QByteArray& json);    ///< Converts json to data
};
}

#endif
