/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _Httpd_H_
#define _Httpd_H_

#include <QObject>
#include "httpserver.h"

namespace HobrasoftHttpd {
class HttpServer;
class HttpConnection;
class HttpRequestHandler;
}

namespace Example {

/**
 * @brief Name space for own application HTTP server
 */
namespace Httpd {

/**
 * @brief Own implementation of HTTP server. 
 *
 * It extends the general class HobrasoftHttpd::HttpServer.
 */
class Httpd : public HobrasoftHttpd::HttpServer {
    Q_OBJECT
  public:

    /**
     * @brief Constructor
     */
    Httpd(QObject *parent);

    /**
     * @brief Returns pointer to new instance of my own request handler HobrasoftHttpd::HttpRequestHandler
     */
    virtual HobrasoftHttpd::HttpRequestHandler *requestHandler(HobrasoftHttpd::HttpConnection *);

  private:

};

}
}

#endif
