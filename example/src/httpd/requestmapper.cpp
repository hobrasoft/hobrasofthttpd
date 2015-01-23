/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "requestmapper.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "staticfilecontroller.h"
#include "httpd/controllerexample.h"

using namespace HobrasoftHttpd;
using namespace Example::Httpd;


RequestMapper::~RequestMapper() {
}


RequestMapper::RequestMapper(HobrasoftHttpd::HttpConnection *parent) : HobrasoftHttpd::HttpRequestHandler(parent) {
}


void RequestMapper::service(HttpRequest *request, HttpResponse *response) {
    QString path = request->path();

    #define ROUTER(adresa, trida) \
        if (path.startsWith(adresa)) { \
            HttpRequestHandler *controller = new trida (connection()); \
            controller->service(request, response); \
            return; \
            }

    ROUTER("/example", ControllerExample);

    HttpRequestHandler::service(request, response);
    response->flush();

}

