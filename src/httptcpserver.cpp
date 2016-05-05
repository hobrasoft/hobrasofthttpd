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


/**
 * @brief Constructor creates the class instance
 */
HttpTcpServer::HttpTcpServer(HttpServer *parent) : QTcpServer(parent) {
    m_settings = parent->settings();
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
    if (!m_settings->useSSL()) {
        QTcpServer::incomingConnection(socketDescriptor);
        return;
        }

    /**
     * If SSL is on then the original QTcpServer::incomingConnection() is unuseable
     * and  SSL must be initialized.
     */
    QSslSocket *socket = new QSslSocket;
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "setSocketDescriptor failed";
        delete socket;
        return;
        }

    /**
     * Signals of the QSslSocket are connected to slots to
     * watch for SSL  errors: slotSslErrors() and slotPeerVerifyError()..
     * When the SSL handshake is complete and the connection is encrypted() then
     * the slotEncrypted() is called and signal newConnection() is emited from that slot.
     */
    m_verified[socket] = true;
    connect(socket, SIGNAL(encrypted()),
            this,     SLOT(slotEncrypted()));
    connect(socket, SIGNAL(          sslErrors(const QList<QSslError>&)),
            this,     SLOT(      slotSslErrors(const QList<QSslError>&)));
    connect(socket, SIGNAL(    peerVerifyError(const QSslError&)),
            this,     SLOT(slotPeerVerifyError(const QSslError&)));
    connect(socket, SIGNAL(    disconnected()),
            this,     SLOT(slotDisconnected()));

    /**
     * FIX: this code should load all certificates in the chain
     * not only the first one.
     */
    QList<QSslCertificate> cacerts;
    QFile cacertf(m_settings->sslCaCrt());
    if (cacertf.open(QIODevice::ReadOnly)) {
        cacerts << QSslCertificate(&cacertf);
        cacertf.close();
        }

    /**
     * Sets the private key, local certificate and 
     * CA certificates chain to the QSslSocket.
     */
    socket->setPrivateKey(m_settings->sslKey());
    socket->setLocalCertificate(m_settings->sslCrt());
    socket->setCaCertificates(cacerts);
    socket->startServerEncryption();

}


/**
 * @brief Slot is invoked when the socket disconnets
 *
 * Removes socket verified status and peer's certificate
 * from inner containers.
 */
void HttpTcpServer::slotDisconnected() {
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    m_verified.remove(socket);
    m_peerCert.remove(socket);
}


/**
 * @brief Slot is invoked when the SSL hanshake is complete and connection is established
 *
 * Each socket and its peer's certificate is registered in m_peerCert.
 * Then newConnectio() signal is emited.
 *
 * Compilation is different in various version of Qt.
 * In earlier Qt versions prior to 4.7 the QSslSocket missed the addPendingConnection()
 * method. I'm not sure if the ssl works properly in older Qt versions.
 */
void HttpTcpServer::slotEncrypted() {
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());

    QSslCertificate crt = socket->peerCertificate();
    m_peerCert[socket] = crt;

    #if QT_VERSION > 0x040700
    addPendingConnection(socket);
    #endif
    emit newConnection();
}


/**
 * @brief Returns true if the peer's certificate is valid and signed with server's CA certificate 
 */
bool HttpTcpServer::verified(QTcpSocket *socket) const {
    if (!m_verified.contains(socket)) { return false; }
    return m_verified[socket];
}


/**
 * @brief Returns peer's certificate
 */
QSslCertificate HttpTcpServer::peerCertificate(QTcpSocket *socket) const {
    if (!m_peerCert.contains(socket)) { return QSslCertificate(); }
    return m_peerCert[socket];
}


/**
 * @brief Slot is called when the peer's certificate is not verified
 */
void HttpTcpServer::slotPeerVerifyError(const QSslError& error) {
    Q_UNUSED(error);
    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    m_verified[socket] = false;
}


/**
 * @brief Slot is called when an SSL error occured
 */
void HttpTcpServer::slotSslErrors(const QList<QSslError>& errors) {
    QList<QSslError> ignoreList;
    for (int i=0; i<errors.size(); i++) {
        if (m_settings->ignoreSslError(errors[i])) {
            ignoreList << errors[i];
            continue;
            }
        qDebug() << "sslError" << errors[i].errorString();
        }

    QSslSocket *socket = qobject_cast<QSslSocket *>(sender());
    socket->ignoreSslErrors(ignoreList);

}


