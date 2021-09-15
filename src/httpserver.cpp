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
#include <QStringList>

using namespace HobrasoftHttpd;


HttpServer::~HttpServer() {
    StaticFileController::deleteMimeTypes();
}


HttpServer::HttpServer(QObject *parent) : QObject(parent) {
    m_settings = new HttpSettings(this);
    start();
}


HttpServer::HttpServer(const HttpSettings* settings, QObject *parent) : QObject(parent) {
    m_settings = settings;
    start();
}


void HttpServer::close() {
    for (int i=0; i<m_servers.size(); i++) {
        m_servers[i]->close();
        delete m_servers[i];
        }
    m_servers.clear();
}


void HttpServer::start() {
    QString address = m_settings->address();

    close();
    QStringList addresses = address.split(QRegExp("\\s+"));
    for (int i=0; i<addresses.size(); i++) {
        HttpTcpServer *server = new HttpTcpServer(this);
        connect(server, SIGNAL(    newConnection(QPointer<HttpConnection>)),
                this,     SLOT(slotNewConnection(QPointer<HttpConnection>)));

        QHostAddress addr;
        int port = m_settings->port();
        if (addresses[i].startsWith("[")) {
            QStringList a = addresses[i].split(QString("]:"));
            addr = QHostAddress(a[0].replace("[", "").replace("]", ""));
            if (a.size() == 2) {
                addr = QHostAddress(a[0].replace("[", ""));
                port = a[1].toInt();
                }
          } else {
            QStringList a = addresses[i].split(QString(":"));
            // IPv4
            if (a.size() == 1) {
                addr = QHostAddress(a[0]);
                }
            // IPv4 + port
            if (a.size() == 2) {
                addr = QHostAddress(a[0]);
                port = a[i].toInt();
                }
            // IPv6
            if (a.size() > 2) {
                addr = QHostAddress(a[0]);
                }
            }

        server->listen(addr, port);

        if (!server->isListening()) {
            qWarning("HttpServer cannot bind on %s:%i : %s", 
                        qPrintable(addr.toString()),
                        port, 
                        qPrintable(server->errorString())
                        );
          } else {
            qDebug("HttpServer listening on %s port %i",
                        qPrintable(addr.toString()),
                        port
                        );
            }
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

