/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "httptcpserver.h"
#include "httpserver.h"
#include "httpsettings.h"
#include <QSslSocket>
#include <QDebug>

using namespace HobrasoftHttpd;


HttpTcpServer::HttpTcpServer(HttpServer *parent) : QTcpServer(parent) {
    m_settings = parent->settings();
}


void HttpTcpServer::incomingConnection(int socketDescriptor) {
    if (!m_settings->useSSL()) {
        QTcpServer::incomingConnection(socketDescriptor);
        return;
        }

    // SSL part
    QSslSocket *socket = new QSslSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "setSocketDescriptor failed";
        delete socket;
        return;
        }

    connect(socket, SIGNAL(encrypted()),
            this,     SLOT(slotEncrypted()));
    connect(socket, SIGNAL(    sslErrors(const QList<QSslError>&)),
            this,     SLOT(slotSslErrors(const QList<QSslError>&)));

    socket->setPrivateKey(m_settings->sslKey());
    socket->setLocalCertificate(m_settings->sslCrt());
    socket->startServerEncryption();

}


void HttpTcpServer::slotEncrypted() {
    qDebug() << "encrypted";
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    addPendingConnection(socket);
    emit newConnection();
}


void HttpTcpServer::slotSslErrors(const QList<QSslError>& errors) {
    qDebug() << "sslErrors";
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    socket->ignoreSslErrors(errors);
}


