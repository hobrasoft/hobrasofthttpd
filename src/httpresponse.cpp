/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpresponse.h"
#include "httprequest.h"
#include "httpconnection.h"
#include "httpgzipcompression.h"
#include <QStringList>

#include <QDebug>

using namespace HobrasoftHttpd;


HttpResponse::~HttpResponse() {
}


HttpResponse::HttpResponse(HttpConnection *connection) : QObject(connection) {
    m_connection = connection;
    m_socket = m_connection->socket();
    m_flushed = false;
    m_statusCode = 200;
    m_statusText = "OK";
    m_sentHeaders = false;
    m_dataBodyPointer = 0;
    m_dataHeadersPointer = 0;
    m_canWriteToSocket = false;
    m_closeAfterFlush = false;
    m_deleteAfterFlush = false;
    m_writerTimer = new QTimer(this);
    m_writerTimer->setInterval(1000);
    m_writerTimer->setSingleShot(true);
    connect (m_writerTimer, SIGNAL(timeout()),
             this,            SLOT(slotWrite()));
    connect (m_socket, SIGNAL(bytesWritten(qint64)),
             this,            SLOT(slotWrite()));
}


bool HttpResponse::isConnected() const {
    return m_connection->isConnected();
}


void HttpResponse::setHeader(const QString& name, const QString& value) {
    if (m_sentHeaders) { return; }
    m_headers[name] = value;
}


void HttpResponse::setHeader(const QString& name, int value) {
    if (m_sentHeaders) { return; }
    m_headers[name] = QString("%1").arg(value);
}


QMap<QString, QString>& HttpResponse::headers() {
    return m_headers;
}


void HttpResponse::clearHeaders() {
    m_headers.clear();
}


void HttpResponse::setStatus(int statusCode, const QString& statusText) {
    m_statusCode = statusCode;
    m_statusText = statusText;
}


void HttpResponse::writeHeaders() {
    if (m_sentHeaders) { return; }

    QString contentType = m_headers.value("Content-Type").toLower();
    bool cancompress = (
        contentType.startsWith("text/plain") ||
        contentType.startsWith("text/html") ||
        contentType.startsWith("text/css") ||
        contentType.startsWith("application/javascript")
        );

    bool requestGzip = (
        m_connection->request() != NULL &&
        m_connection->request()->header("Accept-Encoding").contains("gzip")
        );

    bool chunked = m_headers.value("Transfer-Encoding").toLower() == "chunked" ;

    bool c200 = (m_statusCode == 200);

    if (cancompress && requestGzip && !chunked && c200) {
        setHeader("Content-Encoding", "gzip");
        }

    // int dbs = m_dataBody.size();
    if (m_headers.value("Content-Encoding").toLower() == "gzip" ) {
        m_dataBody = HttpGZipCompression::compressData(m_dataBody);
        }

    m_headers["Content-Length"] = QString("%1").arg(m_dataBody.size());
    /*
    qDebug() << "gzip" << cancompress << requestGzip << !chunked << m_headers.value("Content-Type") << dbs << m_dataBody.size()
            << ( (m_connection->request() != NULL) ? m_connection->request()->path() : "") 
            << ( (m_connection->request() != NULL) ? m_connection->request()->header("Accept-Encoding") : "") 
            << contentType
            ;
    */

    m_dataHeaders += "HTTP/1.1 ";
    m_dataHeaders += QByteArray::number(m_statusCode);
    m_dataHeaders += " ";
    m_dataHeaders += m_statusText;
    m_dataHeaders += "\r\n";

    QStringList keys = m_headers.keys();
    for (int i=0; i<keys.size(); i++) {
        m_dataHeaders += keys[i].toUtf8();
        m_dataHeaders += ": ";
        m_dataHeaders += m_headers[keys[i]].toUtf8();
        m_dataHeaders += "\r\n";
        }

    keys = m_cookies.keys();
    for (int i=0; i<keys.size(); i++) {
        m_dataHeaders += "Set-Cookie: ";
        m_dataHeaders += m_cookies[keys[i]].toByteArray();
        m_dataHeaders += "\r\n";
        }

    m_dataHeaders += "\r\n";
    m_sentHeaders = true;

}


void HttpResponse::setCookie(const HttpCookie& cookie) {
    if (cookie.name().isEmpty()) return;
    m_cookies[cookie.name()] = cookie;
}


void HttpResponse::flushSocket() {
    if (!isConnected()) { return; }
    slotWrite();
    m_socket->flush();
    return;
}


void HttpResponse::close() {
    if (!isConnected()) { return; }
    bool chunked = m_headers.value("Transfer-Encoding").toLower() == "chunked" ;
    if (chunked) {
        m_socket->write("0\r\n\r\n");
        }
    m_socket->flush();
    m_socket->waitForBytesWritten(10000);
    m_socket->disconnectFromHost();
}


void HttpResponse::write(const QByteArray& data) {
    bool chunked = m_headers.value("Transfer-Encoding").toLower() == "chunked" ;
    if (chunked && data.size() > 0) {
        if (!m_sentHeaders) {
            writeHeaders();
            }
        m_dataBody += QByteArray::number(data.size(),16) ;
        m_dataBody += "\r\n";
        m_dataBody += data;
        m_dataBody += "\r\n";
        m_canWriteToSocket = true;
        m_writerTimer->setInterval(0);
        m_writerTimer->start();
        return;
        }

    if (!chunked) {
        if (m_flushed) {
            qDebug() << "You could not write to HttpRespose when the response is flushed. Data written are ignored.";
            return;
            }
        m_closeAfterFlush = true;
        m_dataBody += data;
        }

}


void HttpResponse::flush() {
    m_flushed = true;
    if (!isConnected()) { return; }
    m_canWriteToSocket = true;
    m_writerTimer->setInterval(0);
    m_writerTimer->start();
}


void HttpResponse::flushAndClose() {
    m_closeAfterFlush = true;
    flush();
}


void HttpResponse::flushAndDelete() {
    m_deleteAfterFlush = true;
    flush();
}


void HttpResponse::slotWrite() {
    if (m_headers.size() > 0 && !m_sentHeaders) {
        m_canWriteToSocket = true;
        writeHeaders();
        }
    if (!m_canWriteToSocket) { goto konec; }
    if (m_socket->bytesToWrite() > 0) { goto konec; }
    if (!isConnected()) { goto konec; }
    if (m_socket->isOpen() != true) { goto konec; }
    if (m_socket->isWritable() != true) { goto konec; }
    if (m_dataHeaders.size() > m_dataHeadersPointer) {
        m_dataHeadersPointer += m_socket->write(m_dataHeaders.mid(m_dataHeadersPointer));
        if (m_dataHeaders.size() > m_dataHeadersPointer) { goto konec; }
        }

    if (m_dataBody.size() > m_dataBodyPointer) {
        m_dataBodyPointer += m_socket->write(m_dataBody.mid(m_dataBodyPointer));
        if (m_dataBody.size() > m_dataBodyPointer) { goto konec; }
        }

    if (m_dataHeaders.size() <= m_dataHeadersPointer &&
        m_dataBody.size() <= m_dataBodyPointer &&
        m_closeAfterFlush) {
        m_socket->flush();
        m_writerTimer->stop();
        close();
        return;
        }

    if (m_dataHeaders.size() <= m_dataHeadersPointer &&
        m_dataBody.size() <= m_dataBodyPointer &&
        m_deleteAfterFlush) {
        m_socket->flush();
        m_writerTimer->stop();
        deleteLater();
        return;
        }

  konec:
    m_writerTimer->setInterval(100);
    m_writerTimer->start();
}


