/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httprequesthandler.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "staticfilecontroller.h"
#include "shtmlcontroller.h"

using namespace HobrasoftHttpd;


HttpRequestHandler::HttpRequestHandler(HttpConnection *parent) : QObject(parent) {
    m_connection = parent;
}


void HttpRequestHandler::service(HttpRequest *request, HttpResponse *response) {
    QString path = request->path();
    m_connection->setObjectName(path);

    if (path.endsWith(".shtml")) {
        ShtmlController(m_connection).service(request,response);
        return;
        }

    StaticFileController(m_connection).service(request, response);
}


HttpResponse *HttpRequestHandler::response() {
    return m_connection->response();
}



const HttpSettings *HttpRequestHandler::settings() const { 
    return connection()->settings(); 
}
