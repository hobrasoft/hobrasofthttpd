/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpconnection.h"
#include "httprequest.h"
#include "httprequesthandler.h"
#include "httpresponse.h"
#include "httpserver.h"
#include "httpsettings.h"
#include <QTcpSocket>

using namespace HobrasoftHttpd;

HttpConnection::~HttpConnection() {
    close();
}


HttpConnection::HttpConnection(HttpServer *parent, QTcpSocket *socket) : QObject(parent) {
    m_socket = socket;
    m_request = NULL;
    m_handler = parent->requestHandler(this);
    m_parent  = parent;

    int timeout = parent->settings()->timeout() * 1000;
    m_persistent = parent->settings()->persistent();

    m_timeout = new QTimer(this);
    m_timeout->setInterval(timeout);
    m_timeout->setSingleShot(true);
    m_timeout->start();
    connect (m_timeout, SIGNAL(timeout()),
             this,        SLOT(slotTimeout()));

    connect(m_socket, SIGNAL(readyRead()),
            this,     SLOT(slotRead()));

    connect(m_socket, SIGNAL(disconnected()),
            this,     SLOT(slotDisconnected()));
}

const HttpSettings *HttpConnection::settings() const { 
    return m_parent->settings(); 
}


void HttpConnection::close() {
    if (m_socket) { m_socket->disconnectFromHost(); }
    if (m_request) { delete m_request; }
    m_request = NULL;

}


void HttpConnection::slotTimeout() {
    m_socket->write("HTTP/1.1 408 request timeout\r\n");
    m_socket->write("Connection: close\r\n");
    m_socket->write("\r\n");
    m_socket->write("408 request timeout\r\n");
    m_socket->disconnectFromHost();
    if (m_request) { delete m_request; }
    m_request = NULL;
}


void HttpConnection::slotDisconnected() {
    m_socket->close();
//  m_socket = NULL;
    if (m_request) { delete m_request; }
    m_request = NULL;
    m_timeout->stop();
    QTimer::singleShot(15000, this, SLOT(deleteLater()));
}


HttpResponse HttpConnection::response() {
    m_timeout->start();
    return HttpResponse(m_socket);
}


void HttpConnection::slotRead() {
    m_timeout->start();
    if (m_request == NULL) {
        m_request = new HttpRequest(this);
        }

    while (m_socket->bytesAvailable() 
            && m_request->status() != HttpRequest::StatusComplete
            && m_request->status() != HttpRequest::StatusAbort) {
        m_request->readFromSocket(m_socket);
        if (m_request->status() == HttpRequest::StatusWaitForBody) {
            m_timeout->start();
            }
        }

    if (m_request->status() == HttpRequest::StatusAbort) {
        m_socket->write("HTTP/1.1 413 entity too large\r\n");
        m_socket->write("Connection: close\r\n\r\n");
        m_socket->write("\r\n");
        m_socket->write("413 entity too large\r\n");
        m_socket->disconnectFromHost();
        m_timeout->stop();
        if (m_request) { delete m_request; }
        m_request = NULL;
        return;
        }
    
    if (m_request->status() == HttpRequest::StatusComplete) {
        bool disconnect = (m_request->header("Connection").toLower() == "close");
        m_timeout->stop();
        HttpResponse response(m_socket);
        if (!m_persistent) {
            response.setHeader("Connection", "close");
            }
        m_handler->service(m_request, &response);

        response.flushSocket();

        if (disconnect) {
            if (!response.hasSentLastPart()) {
                response.flush();
                }
            m_socket->disconnectFromHost();
          } else {
            m_timeout->start();
            }

        if (!m_persistent) {
            close();
            }
        if (m_request) { delete m_request; }
        m_request = NULL;
        }

}


