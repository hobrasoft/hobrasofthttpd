/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "httptcpserver.h"
#include "httpserver.h"
#include "httpsettings.h"
#include <QSslSocket>
#include <QSslCertificate>
#include <QFile>
#include <QDateTime>
#include <QDebug>

using namespace HobrasoftHttpd;


HttpTcpServer::HttpTcpServer(HttpServer *parent) : QTcpServer(parent) {
    m_settings = parent->settings();
}


void HttpTcpServer::incomingConnection(QINTPTR socketDescriptor) {
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

    m_verified[socket] = true;
    connect(socket, SIGNAL(encrypted()),
            this,     SLOT(slotEncrypted()));
    connect(socket, SIGNAL(          sslErrors(const QList<QSslError>&)),
            this,     SLOT(      slotSslErrors(const QList<QSslError>&)));
    connect(socket, SIGNAL(    peerVerifyError(const QSslError&)),
            this,     SLOT(slotPeerVerifyError(const QSslError&)));
    connect(socket, SIGNAL(    disconnected()),
            this,     SLOT(slotDisconnected()));


    QList<QSslCertificate> cacerts;
    QFile cacertf(m_settings->sslCaCrt());
    if (cacertf.open(QIODevice::ReadOnly)) {
        cacerts << QSslCertificate(&cacertf);
        cacertf.close();
        }

    socket->setPrivateKey(m_settings->sslKey());
    socket->setLocalCertificate(m_settings->sslCrt());
    socket->setCaCertificates(cacerts);
    socket->startServerEncryption();

}


void HttpTcpServer::slotDisconnected() {
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    m_verified.remove(socket);
    m_peerCert.remove(socket);
}



void HttpTcpServer::slotEncrypted() {
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());

    QSslCertificate crt = socket->peerCertificate();
    m_peerCert[socket] = crt;

    #if QT_VERSION > 0x040700
    addPendingConnection(socket);
    #endif
    emit newConnection();
}


bool HttpTcpServer::verified(QTcpSocket *socket) const {
    if (!m_verified.contains(socket)) { return false; }
    return m_verified[socket];
}


QSslCertificate HttpTcpServer::peerCertificate(QTcpSocket *socket) const {
    if (!m_peerCert.contains(socket)) { return QSslCertificate(); }
    return m_peerCert[socket];
}


void HttpTcpServer::slotPeerVerifyError(const QSslError& error) {
    Q_UNUSED(error);
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
//  qDebug() << "sslError" << error.errorString();
    m_verified[socket] = false;
}


void HttpTcpServer::slotSslErrors(const QList<QSslError>& errors) {
    /*
    for (int i=0; i<errors.size(); i++) {
        qDebug() << "sslError" << errors[i].errorString();
        }
    */
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    socket->ignoreSslErrors(errors);
}


