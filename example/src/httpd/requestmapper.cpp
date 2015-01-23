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

// nové API
#include "httpd/controllerexample.h"

using namespace HobrasoftHttpd;
using namespace Httpd;


RequestMapper::~RequestMapper() {
}


RequestMapper::RequestMapper(HobrasoftHttpd::HttpConnection *parent) : HobrasoftHttpd::HttpRequestHandler(parent) {
}


void RequestMapper::service(HttpRequest *request, HttpResponse *response) {
    m_path = request->path();

    #define ROUTER(adresa, trida) \
        if (m_path.startsWith(adresa)) { \
            HttpRequestHandler *controller = new trida (connection()); \
            controller->service(request, response); \
            return; \
            }

    ROUTER("/example", ControllerExample);

    /**
     * Po speciálních požadavcích volá metodu service z rodičovské třídy a tím 
     * obslouží statické soubory a soubory shtml.
     */
    HttpRequestHandler::service(request, response);
    response->flush();

}

