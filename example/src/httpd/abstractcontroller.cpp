/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "abstractcontroller.h"
#include "httprequesthandler.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "json.h"
#include <QString>
#include <QStringList>


using namespace Example::Httpd;


AbstractController::AbstractController(HobrasoftHttpd::HttpConnection *parent) : HobrasoftHttpd::HttpRequestHandler(parent) {
    m_connection = parent;
    m_checkId = true;
}


HobrasoftHttpd::HttpRequest *AbstractController::request() const { 
    return m_request; 
}


HobrasoftHttpd::HttpConnection *AbstractController::connection() const {
    return m_connection;
}


void AbstractController::service(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    m_request = request;
    QString path = request->path();
    QStringList parts = path.split("/");

    if (parts.size() <= 1) {
        serviceError(request, response, 404, "not-found", "Object not found");
        return;
        }

    parts.removeFirst();

    switch (parts.size()) {
        case 1:
            if (request->method() != "GET") {
                serviceError(request, response, 405, "bad-request", "Method not allowed");
                return;
                }
            serviceList(request, response);
            return;

        case 2:
            if (parts[1] == "events") {
                if (request->method() != "GET") {
                    serviceError(request, response, 405, "bad-request", "Method not allowed");
                    return;
                    }
                response->setHeader("Content-Type",      "text/event-stream");
                response->setHeader("Cache-Control",     "no-cache,public");
                response->setHeader("Transfer-Encoding", "chunked");
                response->flushSocket();
                serviceEvents(request, response);
                return;
                }

            if (parts[1] != "events") {
                QString id = parts[1];
                if (request->method() == "GET") {
                    response->setHeader("Content-Type",  "application/json");
                    response->setHeader("Cache-Control", "no-cache,public");
                    if (!exists(id)) {
                        serviceError(request, response, 404, "not-found", "Not found");
                        return;
                        }
                    serviceIdGet(request, response, id);
                    return;
                    }
                if (request->method() == "PUT") {
                    response->setHeader("Content-Type",  "application/json");
                    response->setHeader("Cache-Control", "no-cache,public");

                    QVariantMap data = JSON::data(request->body()).toMap();
                    if (m_checkId && !data.contains("id")) {
                        serviceError(request, response, 400, "bad-request", "Data must contain ID");
                        return; 
                        } 

                    serviceIdPut(request, response, data);
                    return;
                    }
                if (request->method() == "POST") {
                    response->setHeader("Content-Type",  "application/json");
                    response->setHeader("Cache-Control", "no-cache,public");

                    QVariantMap data = JSON::data(request->body()).toMap();
                    if (m_checkId && !data.contains("id")) {
                        serviceError(request, response, 400, "bad-request", "Data must contain ID");
                        return; 
                        } 

                    serviceIdPost(request, response, data);
                    return;
                    }
                if (request->method() == "DELETE") {
                    response->setHeader("Content-Type",  "application/json");
                    response->setHeader("Cache-Control", "no-cache,public");
                    if (!exists(id)) {
                        serviceError(request, response, 404, "not-found", "Not found");
                        return;
                        }
                    serviceIdDelete(request, response, id);
                    return;
                    }
                }

        case 3:
            if (parts[1] == "events" || parts[2] != "events") {
                serviceError(request, response, 404, "not-found", "Not found");
                return;
                }

            if (request->method() != "GET") {
                serviceError(request, response, 405, "bad-request", "Method not allowed");
                return;
                }

            if (!exists(parts[1])) {
                serviceError(request, response, 404, "not-found", "Not found");
                return;
                }

            response->setHeader("Content-Type",      "text/event-stream");
            response->setHeader("Cache-Control",     "no-cache,public");
            response->setHeader("Transfer-Encoding", "chunked");
            response->flushSocket();
            serviceIdEvents(request, response, parts[1]);
            return;

        default:
            serviceError(request, response, 400, "bad-request", "Request could not be recognized");
            return;
        }
}

void AbstractController::serviceError(
        HobrasoftHttpd::HttpRequest *request, 
        HobrasoftHttpd::HttpResponse *response, 
        int code, 
        const QString& error, 
        const QString& reason) {

    Q_UNUSED(request);

    QVariantMap data;
    data["error"]  = error;
    data["reason"] = reason;

    response->clearHeaders();
    response->setHeader("Content-Type",  "application/json");
    response->setHeader("Cache-Control", "no-cache,public");
    response->setStatus(code, reason);
    response->write( JSON::json(data) );
    response->flush();
}


void AbstractController::serviceOK (
        HobrasoftHttpd::HttpRequest *request, 
        HobrasoftHttpd::HttpResponse *response, 
        const QVariant& data) {

    Q_UNUSED(request);

    QVariantMap okdata;
    okdata["ok"] = true;

    if (!response->containsHeader("Content-Type"))  { response->setHeader("Content-Type",  "application/json"); }
    if (!response->containsHeader("Cache-Control")) { response->setHeader("Cache-Control", "no-cache,public"); }
    response->setStatus(200, "OK");
    response->write( JSON::json( (!data.isValid() || data.isNull()) ? okdata : data) );
    response->flush();
}


void AbstractController::serviceEvent (
        HobrasoftHttpd::HttpRequest *request, 
        HobrasoftHttpd::HttpResponse *response, 
        const QVariant& data) { 
    Q_UNUSED(request);
    Q_UNUSED(response);

    connection()->setTimeout(0);
    HobrasoftHttpd::HttpResponse *lresponse = connection()->response();
    lresponse->setHeader("Transfer-Encoding", "chunked");
    lresponse->setHeader("Content-Type",      "text/event-stream");
    lresponse->setHeader("Cache-Control",     "no-cache,public");
    lresponse->setSendHeaders(false);
    
    QByteArray datagram;
    datagram += "event: status\n";
    datagram += "data: ";
    datagram += JSON::json(data);
    datagram += "\n";
    datagram += "\n";
    lresponse->write(datagram);
    lresponse->flushSocket();
    lresponse->flushAndDelete();
}

void AbstractController::serviceList      (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}


void AbstractController::serviceEvents    (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}


void AbstractController::serviceIdEvents  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString&) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}


void AbstractController::serviceIdDelete  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString&) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}


void AbstractController::serviceIdGet     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString&) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}


void AbstractController::serviceIdPut     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariantMap&) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}


void AbstractController::serviceIdPost    (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariantMap&) {
    serviceError(request, response, 501, "not-implemeted", "Method not implemented");
}

