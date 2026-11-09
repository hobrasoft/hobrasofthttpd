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
#include "httpthread.h"
#include <QTcpSocket>
#include <QFileInfo>
#ifdef HOBRASOFT_HTTPD_SSL
#include <QSslKey>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslConfiguration>
#endif

#include <QDebug>

using namespace HobrasoftHttpd;

HttpConnection::~HttpConnection() {
    close();
    for (int i=0; i<m_requests.size(); i++) {
        delete m_requests[i];
        }
}


QPointer<HttpConnection> HttpConnection::createConnection(HttpServer *parent, QINTPTR socketDescriptor) {
    QPointer<HttpConnection> connection;
    connection = new HttpConnection(parent, socketDescriptor);
    if (parent->settings()->thread()) {
        connection->setParent(NULL);
        HttpThread *thread = new HttpThread(parent);
        thread->setObjectName("Http connection");
        thread->start();
        connection->moveToThread(thread);
        connect(connection, SIGNAL(destroyed(QObject *)),
                thread,       SLOT(quit()));
        connect(thread, SIGNAL(finished()),
                thread,   SLOT(deleteLater()));
        }
    QMetaObject::invokeMethod(connection, "slotCreateSocket", Qt::QueuedConnection);
    return connection;
}


HttpConnection::HttpConnection(HttpServer *parent, QINTPTR socketDescriptor) : QObject(parent) {
    m_socketDescriptor = socketDescriptor;
    m_socket = NULL;
    m_parent = parent;
}


void HttpConnection::slotCreateSocket() {
#ifdef HOBRASOFT_HTTPD_SSL
    if (!settings()->useSSL()) {
#endif
        m_socket = new QTcpSocket(this);
        m_socket->setSocketDescriptor(m_socketDescriptor);
#ifdef HOBRASOFT_HTTPD_SSL
        }

    if (settings()->useSSL()) {
        m_socket = new QSslSocket(this);
        m_socket->setSocketDescriptor(m_socketDescriptor);

        /**
         * If SSL is on then the original QTcpServer::incomingConnection() is unuseable
         * and  SSL must be initialized.
         */
        QSslSocket *socket = new QSslSocket;
        if (!socket->setSocketDescriptor(m_socketDescriptor)) {
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
        m_verified = true;
        connect(socket, SIGNAL(encrypted()),
                this,     SLOT(slotEncrypted()));
        #if QT_VERSION > 0x050000
        connect(socket, SIGNAL(          sslErrors(const QList<QSslError>&)),
                this,     SLOT(      slotSslErrors(const QList<QSslError>&)));
        connect(socket, SIGNAL(    peerVerifyError(const QSslError&)),
                this,     SLOT(slotPeerVerifyError(const QSslError&)));
        #endif
        connect(socket, SIGNAL(    disconnected()),
                this,     SLOT(slotDisconnected()));

        /**
         * FIX: this code should load all certificates in the chain
         * not only the first one.
         */
/*
        QFile cacertf(settings()->sslCaCrt());
        if (cacertf.open(QIODevice::ReadOnly)) {
            cacerts << QSslCertificate(&cacertf);
            cacertf.close();
            }
*/

        QList<QSslCertificate> caCertificates;

        if (!settings()->sslCaCrt().isEmpty() &&
            QFileInfo::exists(settings()->sslCaCrt())) {
            QList<QSslCertificate> caCertificates;
            QFile fileca(settings()->sslCaCrt());
            fileca.open(QIODevice::ReadOnly);
            caCertificates << QSslCertificate(&fileca);
            fileca.close();
            if (caCertificates.isEmpty()) {
                qDebug() << "No CA certificate loaded!";
                }
            }

        #if QT_VERSION > 0x060000
        QSslConfiguration sslConfiguration = QSslConfiguration::defaultConfiguration();
        sslConfiguration.setCaCertificates(caCertificates);

        if (!settings()->sslKey().isEmpty() &&
            QFileInfo::exists(settings()->sslKey())) {
            QFile filekey(settings()->sslKey());
            filekey.open(QIODevice::ReadOnly);
            QSslKey key = QSslKey(&filekey, QSsl::Rsa);
            sslConfiguration.setPrivateKey(key);
            if (key.isNull()) {
                qDebug() << "SSL key isNull" << filekey.fileName();
                }
            }

        if (!settings()->sslCrt().isEmpty() && 
            QFileInfo::exists(settings()->sslCrt())) {
            QFile filecrt(settings()->sslCrt());
            filecrt.open(QIODevice::ReadOnly);
            QSslCertificate crt = QSslCertificate(&filecrt);
            sslConfiguration.setLocalCertificate(crt);
            if (crt.isNull()) {
                qDebug() << "SSL certifikat isNull" << filecrt.fileName();
                }
            }


        socket->setSslConfiguration(sslConfiguration);
        #else
        /**
         * Sets the private key, local certificate and 
         * CA certificates chain to the QSslSocket.
         */
        socket->setPrivateKey(settings()->sslKey());
        socket->setLocalCertificate(settings()->sslCrt());
        socket->setCaCertificates(caCertificates);
        socket->startServerEncryption();
        #endif

        }
#endif

    m_peerAddress = m_socket->peerAddress();
    m_request = NULL;
    m_handler = m_parent->requestHandler(this);
    m_connected = true;
    m_inService = true;

    int timeout = settings()->timeout() * 1000;

    m_timeout = new QTimer(this);
    m_timeout->setInterval(timeout);
    m_timeout->setSingleShot(true);
    startTimeout();
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


void HttpConnection::startTimeout() {
    if (m_timeout->interval() == 0) {
        m_timeout->stop();
        return;
        }
    m_timeout->start();
}


#ifdef HOBRASOFT_HTTPD_SSL
void HttpConnection::setPeerCertificate(const QSslCertificate& crt) {
    m_peerCertificate = crt;
}
#endif


#ifdef HOBRASOFT_HTTPD_SSL
QString HttpConnection::commonName() const {
    #if QT_VERSION > 0x050000
    QDateTime now = QDateTime::currentDateTime();
    if (now > m_peerCertificate.expiryDate())    { return QString(); }
    if (now < m_peerCertificate.effectiveDate()) { return QString(); }
    if (m_peerCertificate.isBlacklisted()) { return QString(); }
    QStringList cn = m_peerCertificate.subjectInfo(QSslCertificate::CommonName);
    if (cn.isEmpty()) { return QString(); }
    return cn.first();
    #else
    if (!m_peerCertificate.isValid()) { return QString(); }
    return m_peerCertificate.subjectInfo(QSslCertificate::CommonName);
    #endif
}
#endif


void HttpConnection::close() {
    if (!isConnected()) { 
        return; 
        }
    if (m_socket) { 
        m_socket->disconnectFromHost(); 
        }
    deleteRequest();

}


void HttpConnection::slotTimeout() {
    if (!isConnected()) { return; }
    m_socket->write("HTTP/1.1 408 request timeout\r\n");
    m_socket->write("Connection: close\r\n");
    m_socket->write("\r\n");
    m_socket->write("408 request timeout\r\n");
    m_socket->disconnectFromHost();
    deleteRequest();
}


void HttpConnection::slotDisconnected() {
    m_connected = false;
    m_socket->close();
    deleteRequest();
    m_timeout->stop();
    if (m_inService) {
        QTimer::singleShot(3000, this, SLOT(deleteLater()));
      } else {
        deleteLater();
        }
}


HttpResponse *HttpConnection::response() {
    startTimeout();
    return new HttpResponse(this);
}


void HttpConnection::slotRead() {
    if (!isConnected()) { return; }
    startTimeout();
    if (m_request == NULL || (m_request != NULL && m_request->status() == HttpRequest::StatusComplete)) {
        m_request = new HttpRequest(this);
        m_requests << m_request;
        }

    while (m_socket->bytesAvailable() 
            && m_request->status() != HttpRequest::StatusComplete
            && m_request->status() != HttpRequest::StatusAbort) {
        m_request->readFromSocket(m_socket);
        if (m_request->status() == HttpRequest::StatusWaitForBody) {
            startTimeout();
            }
        }

    if (m_request->status() == HttpRequest::StatusAbort) {
        m_socket->write("HTTP/1.1 413 entity too large\r\n");
        m_socket->write("Connection: close\r\n\r\n");
        m_socket->write("\r\n");
        m_socket->write("413 entity too large\r\n");
        m_socket->disconnectFromHost();
        m_timeout->stop();
        deleteRequest();
        return;
        }
    
    if (m_request->status() == HttpRequest::StatusComplete) {
        bool disconnect = (m_request->header("Connection").toLower() == "close");
        m_timeout->stop();
        HttpResponse *response = new HttpResponse(this);
        m_inService = true;
        m_handler->service(m_request, response);
        m_inService = false;

        if (!isConnected()) { 
            deleteLater();
            return; 
            }

        if (disconnect) {
            if (isConnected()) { 
                m_socket->disconnectFromHost(); 
                deleteRequest();
                }
          } else {
            startTimeout();
            }

        }

}


void HttpConnection::deleteRequest() {
    m_request = NULL;
}


QVariant HttpConnection::webStatus() const {
    QVariantList list;
    for (int i=0; i<m_requests.size(); i++) {
        const HttpRequest *request = m_requests[i];
        QVariantMap data;
        data["object"]      = QString("0x%1").arg((quint64)this, 8, 16, QChar('0'));
        data["path"]        = (request!=NULL) ? request->path() : QVariant();
        data["time"]        = (request!=NULL) ? request->datetime() : QVariant();
        data["method"]      = (request!=NULL) ? request->method() : QVariant();
        data["status"]      = (request!=NULL) ? request->statusString() : QVariant();
        data["connection"]  = (isConnected()) ? "connected" : "disconnected";
        list << data;
        }
    return list;
}


#ifdef HOBRASOFT_HTTPD_SSL
void HttpConnection::slotEncrypted() {
    QSslSocket *socket = qobject_cast<QSslSocket *>(m_socket);
    m_peerCertificate = socket->peerCertificate();
}

#if QT_VERSION > 0x050000
/**
 * @brief Slot is called when the peer's certificate is not verified
 */
#if QT_VERSION > 0x060000
void HttpConnection::slotPeerVerifyError(const QSslError::SslError& error) {
#else
void HttpConnection::slotPeerVerifyError(const QSslError& error) {
#endif
#endif
    Q_UNUSED(error);
    m_verified = false;
}


#if QT_VERSION > 0x060000
/**
 * @brief Slot is called when an SSL error occured
 */
void HttpConnection::slotSslErrors(const QList<QSslError::SslError>& errors) {
    QList<QSslError> ignoreList;
    for (int i=0; i<errors.size(); i++) {
        if (settings()->ignoreSslError(errors[i])) {
            ignoreList << QSslError(errors[i]);
            continue;
            }
        // qDebug() << "sslError" << errors[i].errorString();
        }

    QSslSocket *socket = qobject_cast<QSslSocket *>(m_socket);
    socket->ignoreSslErrors(ignoreList);
}

#else
/**
 * @brief Slot is called when an SSL error occured
 */
void HttpConnection::slotSslErrors(const QList<QSslError>& errors) {
    QList<QSslError> ignoreList;
    for (int i=0; i<errors.size(); i++) {
        if (settings()->ignoreSslError(errors[i])) {
            ignoreList << errors[i];
            continue;
            }
        qDebug() << "sslError" << errors[i].errorString();
        }

    QSslSocket *socket = qobject_cast<QSslSocket *>(m_socket);
    socket->ignoreSslErrors(ignoreList);
}

#endif
#endif


