/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpresponse.h"
#include <QStringList>

using namespace HobrasoftHttpd;

HttpResponse::HttpResponse(QTcpSocket *socket) {
    m_socket = socket;
    m_statusCode = 200;
    m_statusText = "OK";
    m_sentHeaders = false;
    m_sentLastPart = false;
}


void HttpResponse::setHeader(const QString& name, const QString& value) {
    if (m_sentHeaders) return;
    m_headers[name] = value;
}


void HttpResponse::setHeader(const QString& name, int value) {
    if (m_sentHeaders) return;
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
    if (m_sentHeaders) return;
    QByteArray buffer;
    buffer += "HTTP/1.1 ";
    buffer += QByteArray::number(m_statusCode);
    buffer += " ";
    buffer += m_statusText;
    buffer += "\r\n";

    QStringList keys = m_headers.keys();
    for (int i=0; i<keys.size(); i++) {
        buffer += keys[i].toUtf8();
        buffer += ": ";
        buffer += m_headers[keys[i]].toUtf8();
        buffer += "\r\n";
        }

    keys = m_cookies.keys();
    for (int i=0; i<keys.size(); i++) {
        buffer += "Set-Cookie: ";
        buffer += m_cookies[keys[i]].toByteArray();
        buffer += "\r\n";
        }

    buffer += "\r\n";
    writeToSocket(buffer);
    m_sentHeaders = true;

}


void HttpResponse::setCookie(const HttpCookie& cookie) {
    if (cookie.name().isEmpty()) return;
    m_cookies[cookie.name()] = cookie;
}


void HttpResponse::writeToSocket(const QByteArray& data) {
    int remaining = data.size();
    const char *ptr = data.data();
    while (m_socket->isOpen() && remaining>0) {
        int written = m_socket->write(data);
//      m_socket->waitForBytesWritten(3000);
        ptr += written;
        remaining -= written;
        }
}


void HttpResponse::flush() {
    write(QByteArray(), true);
    m_socket->flush();
}


void HttpResponse::flushSocket() {
    write(QByteArray(), false);
    m_socket->flush();
}


void HttpResponse::close() {
    m_socket->disconnectFromHost();
}


void HttpResponse::write(const QByteArray& data) {
    write(data, false);
}


void HttpResponse::write(const QByteArray& data, bool lastPart) {
    if (m_sentLastPart) {
        return;
        }
    if (!m_sentHeaders) {
        QString connectionMode = m_headers.value("Connection");
        if (!m_headers.contains("Content-Length") &&
            !m_headers.contains("Transfer-Encoding") &&
            connectionMode.toLower() != "close") {
            if (lastPart) {
                m_headers["Transfer-Encoding"] = "chunked";
              } else {
                m_headers["Content-Length"] = QString("%1").arg(data.size());
                }
            }
        writeHeaders();
        }

    bool chunked = m_headers.value("Transfer-Encoding").toLower() == "chunked" ;
    if (chunked && data.size() > 0) {
        writeToSocket( QByteArray::number(data.size(),16) );
        writeToSocket( "\r\n" );
        writeToSocket( data );
        writeToSocket( "\r\n" );
        }

    if (!chunked) {
        writeToSocket(data);
        }

    if (lastPart) {
        m_sentLastPart = true;

        if (chunked) {
            writeToSocket("0\r\n\r\n");
            }

        if (!m_headers.contains("Content-Length")) {
            m_socket->disconnectFromHost();
            }

        }

}

