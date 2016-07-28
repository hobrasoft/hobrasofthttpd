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
    m_datetime = QDateTime::currentDateTime();
    m_socket = socket;
    m_request = NULL;
    m_handler = parent->requestHandler(this);
    m_parent  = parent;
    m_connected = true;
    m_inService = true;

    int timeout = parent->settings()->timeout() * 1000;

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


void HttpConnection::setPeerCertificate(const QSslCertificate& crt) {
    m_peerCertificate = crt;
}


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


void HttpConnection::close() {
    if (!isConnected()) { return; }
    if (m_socket) { m_socket->disconnectFromHost(); }
    if (m_request) { delete m_request; }
    m_request = NULL;

}


void HttpConnection::slotTimeout() {
    if (!isConnected()) { return; }
    m_socket->write("HTTP/1.1 408 request timeout\r\n");
    m_socket->write("Connection: close\r\n");
    m_socket->write("\r\n");
    m_socket->write("408 request timeout\r\n");
    m_socket->disconnectFromHost();
    if (m_request) { delete m_request; }
    m_request = NULL;
}


void HttpConnection::slotDisconnected() {
    m_connected = false;
    m_socket->close();
    if (m_request) { delete m_request; }
    m_request = NULL;
    m_timeout->stop();
    QTimer::singleShot( (m_inService) ? 30000 : 3000, this, SLOT(deleteLater()));
}


HttpResponse *HttpConnection::response() {
    m_timeout->start();
    return new HttpResponse(this);
}


void HttpConnection::slotRead() {
    if (!isConnected()) { return; }
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
        HttpResponse *response = new HttpResponse(this);
        m_inService = true;
        m_handler->service(m_request, response);
        m_inService = false;

        if (!isConnected()) { 
            deleteLater();
            return; 
            }

        if (disconnect) {
            if (isConnected()) { m_socket->disconnectFromHost(); }
          } else {
            m_timeout->start();
            }

        if (m_request) { delete m_request; }
        m_request = NULL;
        }

}


QVariant HttpConnection::webStatus() const {
    QVariantMap data;
    data["path"] = m_request->path();
    data["time"] = m_datetime;
    data["status"] = (isConnected()) ? "connected" : "disconnected";
    return data;
}


