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
    for (int i=0; i<m_requests.size(); i++) {
        delete m_requests[i];
        }
}


HttpConnection::HttpConnection(HttpServer *parent, QTcpSocket *socket) : QObject(parent) {
    m_peerAddress = socket->peerAddress();
    m_socket = socket;
    m_request = NULL;
    m_parent  = parent;
    m_handler = parent->requestHandler(this);
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
    if (m_request == NULL) {
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


