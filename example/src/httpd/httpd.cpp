/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpd.h"
#include "requestmapper.h"
#include "application.h"


Example::Httpd::Httpd::Httpd(QObject *parent) : HobrasoftHttpd::HttpServer(parent) {
}


HobrasoftHttpd::HttpRequestHandler *Example::Httpd::Httpd::requestHandler(HobrasoftHttpd::HttpConnection *connection) {
    return new RequestMapper(connection);
}

