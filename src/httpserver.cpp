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
#include "staticfilecontroller.h"
#include <QSslSocket>
#include <QPointer>
#include <QThread>

using namespace HobrasoftHttpd;


HttpServer::~HttpServer() {
    StaticFileController::deleteMimeTypes();
}


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
    connect(m_server, SIGNAL(    newConnection(QPointer<HttpConnection>)),
            this,       SLOT(slotNewConnection(QPointer<HttpConnection>)));

    m_server->listen(address, port);

    if (!m_server->isListening()) {
        qWarning("HttpServer cannot bind on %s:%i : %s", 
                    qPrintable(address.toString()), 
                    port, 
                    qPrintable(m_server->errorString())
                    );
      } else {
        /*
        qDebug("HttpServer listening on %s port %i",
                    qPrintable(address.toString()), 
                    port
                    );
        */
        }
}


HttpRequestHandler *HttpServer::requestHandler(HttpConnection *parent) {
    return new HttpRequestHandler(parent);
}


void HttpServer::slotNewConnection(QPointer<HttpConnection> connection) {
    bool threads = m_settings->threads();
    if (threads) {
        m_connections << connection;
        connect(connection, SIGNAL(destroyed(QObject *)),
                this,         SLOT(slotConnectionClosed(QObject *)));
        }
}


void HttpServer::slotConnectionClosed(QObject *object) {
    Q_UNUSED(object);
    // QPointer<HttpConnection> connection = qobject_cast<HobrasoftHttpd::HttpConnection*>(object);
    // HttpConnection *connection = qobject_cast<HobrasoftHttpd::HttpConnection*>(object);
    // m_connections.removeAll(connection);
    m_connections.removeAll(NULL);
}


QVariant HttpServer::webStatus() const {
    QVariantList objectlist;
    QObjectList list = children();
    for (int i=0; i<list.size(); i++) {
        const QObject *qobject = list[i];
        QString classname = qobject->metaObject()->className();
        if (classname != "HobrasoftHttpd::HttpConnection") {
            continue;
            }
        const HttpConnection *connection = qobject_cast<const HttpConnection *>(qobject);
        objectlist += connection->webStatus().toList();
        }

    return objectlist;
}

