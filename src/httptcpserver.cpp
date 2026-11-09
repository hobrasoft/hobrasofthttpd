/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "httptcpserver.h"
#include "httpserver.h"
#include "httpsettings.h"
#include "httpconnection.h"
#include <QFile>
#include <QDateTime>
#include <QDebug>

using namespace HobrasoftHttpd;


/**
 * @brief Constructor creates the class instance
 */
HttpTcpServer::HttpTcpServer(HttpServer *parent) : QTcpServer(parent) {
    m_parent = parent;
    m_settings = parent->settings();
    setMaxPendingConnections(300);
}


/**
 * @brief Method is invoked when incoming connection arrived
 *
 * Overwrites QTctpServer::incomingConnection()
 */
void HttpTcpServer::incomingConnection(QINTPTR socketDescriptor) {
    /**
     * If no SSL is used then the original QTcpServer::incomingConnection() is called.
     */

    // qDebug() << "HttpTcpServer::incomingConnection()" << socketDescriptor;
    QPointer<HttpConnection> connection = HttpConnection::createConnection(m_parent, socketDescriptor);
    emit newConnection(connection);
}

