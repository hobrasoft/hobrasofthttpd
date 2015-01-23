/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpd.h"
#include "requestmapper.h"
#include "application.h"


Httpd::Httpd::Httpd(QObject *parent) : HobrasoftHttpd::HttpServer(parent) {
}


HobrasoftHttpd::HttpRequestHandler *Httpd::Httpd::requestHandler(HobrasoftHttpd::HttpConnection *connection) {
    return new RequestMapper(connection);
}

