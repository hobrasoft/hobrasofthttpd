/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpserver.h"
#include "httpsettings.h"
#include "httpconnection.h"
#include "httprequesthandler.h"
#include "httptcpserver.h"
#include <QSslSocket>

using namespace HobrasoftHttpd;


HttpServer::HttpServer(QObject *parent) : QObject(parent) {
    m_server = NULL;
    m_settings = new HttpSettings(this);
    start();
}


HttpServer::HttpServer(const HttpSettings* settings, QObject *parent) : QObject(parent) {
    m_server = NULL;
    m_settings = settings;
    start();
}


void HttpServer::close() {
    if (m_server != NULL) {
        m_server->close();
        }
}


void HttpServer::start() {
    QHostAddress address = m_settings->address();
    int             port = m_settings->port();
    
    if (m_server != NULL) {
        m_server->close();
        delete m_server;
        }
    m_server = new HttpTcpServer(this);
    connect(m_server, SIGNAL(    newConnection()),
            this,       SLOT(slotNewConnection()));

    m_server->listen(address, port);

    if (!m_server->isListening()) {
        qWarning("HttpServer: cannot bind on %s:%i : %s", 
                    qPrintable(address.toString()), 
                    port, 
                    qPrintable(m_server->errorString())
                    );
        }
}


HttpRequestHandler *HttpServer::requestHandler(HttpConnection *parent) {
    return new HttpRequestHandler(parent);
}


void HttpServer::slotNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        new HttpConnection(this, socket);
        }
}


