/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _StaticFileController_H_
#define _StaticFileController_H_

#include <QObject>
#include <QCache>
#include <QHash>
#include <QDateTime>
#include "httprequesthandler.h"
#include "testsettings.h"

namespace HobrasoftHttpd {

class HttpRequest;
class HttpResponse;
class HttpConnection;
class HttpSettings;


/**
 * @brief Processes request for static files
 *
 * The files can be stored in cache.
 *
 * @see HttpSettings
 */
class StaticFileController : public HttpRequestHandler {
    FRIEND_CLASS_TEST;
    Q_OBJECT
  public:

    /**
     * @brief Constructor set default parameters from configuration
     *
     * The configuration is taken from parent HttpConnection::settings()
     */
    StaticFileController(HttpConnection *parent);

    /**
     * @brief Processes the request
     */
    void service(HttpRequest *request, HttpResponse *response);

    /**
     * @brief Adds mime type to static table of mime types (common for all class instances)
     *
     * These suffixes are recognized by default: png, jpg, jpeg, gif, txt, html, xhtml, htm, css, json, js
     */
    void addMimeType(const QString& fileSuffix, const QString& mimetype);

    static QString toGMTString(const QDateTime&);


  private:
    /**
     * @brief Returs pointer to HttpSettings used in the HttpServer
     */
    const HttpSettings *settings() const;

    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    static QHash<QString, QString> m_mimetypes;
    HttpConnection  *m_parent;
    #endif

};

}

#endif
