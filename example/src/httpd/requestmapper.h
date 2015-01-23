/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _RequestMapper_H_
#define _RequestMapper_H_

#include <QObject>
#include "httprequesthandler.h"

namespace HobrasoftHttpd {
class HttpRequestHandler;
class HttpRequest;
class HttpResponse;
class HttpConnection;
}

namespace Example {
namespace Httpd {

/**
 * @brief Processes HTTP request
 *
 * Special requests to inner data are processed by application.
 * Such services has to be implemented in your own classes.
 *
 * Other general requests (static files) are processed by default
 * implementation in HobrasoftHttpd::HttpRequestHandler.
 *
 */
class RequestMapper : public HobrasoftHttpd::HttpRequestHandler {
    Q_OBJECT
  public:

   ~RequestMapper();

    /**
     * @brief Construktor
     */
    RequestMapper(HobrasoftHttpd::HttpConnection *parent);

    /**
     * @brief Processes the request
     *
     * Own specialized classes are created and called for dynamic content.
     * General request handler is created for other static content requests.
     */
    void service(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

  private:

};

}
}

#endif
