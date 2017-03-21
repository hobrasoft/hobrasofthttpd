/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httprequest.h"
#include "httpconnection.h"
#include "httpsettings.h"
#include <QList>
#include <QDir>
#include <QMapIterator>
#include <QDebug>

using namespace HobrasoftHttpd;


HttpRequest::HttpRequest(HttpConnection *parent) {
    m_datetime = QDateTime::currentDateTime();
    m_status = StatusWaitForRequest;
    m_currentSize = 0;
    m_expectedBodySize = 0;
    m_connection = parent;
}


void HttpRequest::readFromSocket(QTcpSocket *socket) {
    switch(m_status) {
        case StatusComplete:
            return;
            break;
        case StatusAbort:
            return;
            break;
        case StatusWaitForRequest:
            readRequest(socket);
            break;
        case StatusWaitForHeader:
            readHeader(socket);
            break;
        case StatusWaitForBody:
            readBody(socket);
            break;
        };
    if (m_currentSize > m_connection->settings()->maxRequestSize()) {
        m_status = StatusAbort;
        }
    if (m_status == StatusComplete) {
        decodeRequestParams();
        extractCookies();
        }
}


/**
 * @brief Reads request from socket
 *
 * Parses first line of the request and stores the method (GET, PUT, POST...)
 * the path (/path-to-request), version (HTTP/1.1) to class instance.
 */
void HttpRequest::readRequest(QTcpSocket *socket) {
    int toRead = m_connection->settings()->maxRequestSize() - m_currentSize + 1;
    QByteArray newdata = socket->readLine(toRead).trimmed();
    m_currentSize += newdata.size();

    if (newdata.isEmpty()) {
        return;
        }

    QStringList list = QString::fromUtf8(newdata).split(' ');
    if (list.size() != 3 || !list[2].contains("HTTP")) {
        qWarning("HttpRequest: received broken HTTP request, invalid first line");
        m_status = StatusAbort;
        return;
        }

    m_method  = list[0];
    m_path    = list[1];
    m_version = list[2];
    m_status  = StatusWaitForHeader;
    m_fullpath = m_path;
    m_connection->setObjectName(m_path);
}


/**
 * @brief Reads HTTP headers of the request
 */
void HttpRequest::readHeader(QTcpSocket *socket) {
    int toRead = m_connection->settings()->maxRequestSize() - m_currentSize + 1;
    QByteArray newdata = socket->readLine(toRead).trimmed();
    m_currentSize += newdata.size();

    int colonpos = newdata.indexOf(':');
    if (colonpos > 0) { // new header
        m_currentHeader = QString::fromUtf8(newdata.left(colonpos));
        QString value   = QString::fromUtf8(newdata.mid(colonpos+1).trimmed());
        m_headers.insert(m_currentHeader,value);
        return;
        }

    if (colonpos <= 0 && !newdata.isEmpty()) { // header continues
        m_headers.insert(
                m_currentHeader, 
                m_headers.value(m_currentHeader) + QString(" ") + QString::fromUtf8(newdata)
                );
        return;
        }

    if (newdata.isEmpty()) { // Empty line, end of headers
        QString contentType = header("Content-Type");

        if (contentType.startsWith("multipart/form-data", Qt::CaseInsensitive)) {
            int pos  = contentType.indexOf("boundary=", 0, Qt::CaseInsensitive);
            if (pos >= 0) {
                m_boundary = contentType.mid(pos+9).toUtf8();
                }
            }

        m_expectedBodySize = header("Content-Length").toInt();
        if (m_expectedBodySize <= 0) {
            m_status = StatusComplete;
            return;
            }

        if (m_boundary.isEmpty() && m_expectedBodySize + m_currentSize > m_connection->settings()->maxRequestSize()) {
            qWarning("HttpRequest: expected body is too large");
            m_status = StatusAbort;
            return;
            }

        if (!m_boundary.isEmpty() && m_expectedBodySize > m_connection->settings()->maxMultiPartSize()) {
            qWarning("HttpRequest: expected multipart body is too large");
            m_status = StatusAbort;
            return;
            }

        m_status = StatusWaitForBody;
        
        }

}


/**
 * @brief Reads body of the request
 */
void HttpRequest::readBody(QTcpSocket *socket) {
    // normal body, no multipart
    if (m_boundary.isEmpty()) {
        int toRead = m_expectedBodySize - m_bodyData.size();
        QByteArray newdata = socket->read(toRead);
        m_currentSize += newdata.size();

        m_bodyData.append(newdata);
        if (m_bodyData.size() >= m_expectedBodySize) {
            m_status = StatusComplete;
            }
        return;
        }

    // Multipart body
    if (!m_boundary.isEmpty()) {
        if (!m_tempFile.isOpen()) {
            m_tempFile.open();
            }
        int filesize = m_tempFile.size();
        int toRead = m_expectedBodySize - filesize;
        if (toRead > 65536) {
            toRead = 65536;
            }

        filesize += m_tempFile.write(socket->read(toRead));
        if (filesize >= m_connection->settings()->maxMultiPartSize()) {
            qWarning("HttpRequest: received too many multipart bytes");
            m_status = StatusAbort;
            return;
            }

        if (filesize >= m_expectedBodySize) {
            m_tempFile.flush();
            parseMultiPartFile();
            m_tempFile.close();
            m_status = StatusComplete;
            }
        }

}


/**
 * @brief Parses parameters of the URL
 */
void HttpRequest::decodeRequestParams() {
    QString rawParameters;
    if (header("Content-Type").toLower() == "application/x-www-form-urlencoded") {
        rawParameters = m_bodyData;
      } else {
        int questionmarkpos = m_path.indexOf('?');
        if (questionmarkpos >= 0) {
            rawParameters = m_path.mid(questionmarkpos+1);
            m_path = m_path.left(questionmarkpos);
            }
        }

    QStringList list = rawParameters.split('&');
    for (int i=0; i<list.size(); i++) {
        QString& part = list[i];
        int eqpos = part.indexOf('=');
        if (eqpos >= 0) {
            QString name  = urlDecode( part.left(eqpos).trimmed() );
            QString value = urlDecode( part.mid(eqpos+1).trimmed() );
            m_parameters.insert(name, value);
            continue;
            }

        if (!part.isEmpty()) {
            QString name = urlDecode( part );
            m_parameters.insert(name,"");
            }
        }
}


/**
 * @brief Parses cookies 
 */
void HttpRequest::extractCookies() {
    QStringList cookies = headers("Cookie");
    for (int i=0; i<cookies.size(); i++) {
        // rozdělit řádek podle středníků
        QStringList parts = cookies[i].split(";");
        for (int i=0; i<parts.size(); i++) {
            QString& part = parts[i];
            int eqpos = part.indexOf('=');
            if (eqpos >= 0) {
                QString name  = part.left(eqpos).trimmed();
                QString value = part.mid(eqpos+1).trimmed();
                if (name.startsWith("$")) {
                    continue;
                    }
                m_cookies.insert(name, value);
                continue;
                }
            if (eqpos < 0) {
                m_cookies.insert(part, "");
                continue;
                }
            }
        }
    header("Cookie");
}


/**
 * @brief Dedodes % encoding used in URL
 */
QString HttpRequest::urlDecode(const QString& text) {
    QByteArray buffer(text.toUtf8());
    buffer.replace('+',' ');
    int percentpos = buffer.indexOf('%');
    while (percentpos >= 0) {
        bool ok;
        // QChar byte = QChar((int)buffer.mid(percentpos+1, 2).toInt(&ok, 16));
        int byte = buffer.mid(percentpos+1, 2).toInt(&ok, 16);
        if (ok) {
            buffer.remove(percentpos,3);
            buffer.insert(percentpos, byte);
            }
        percentpos = buffer.indexOf('%', percentpos+1);
        }
    return QString::fromUtf8(buffer);
}


/**
 * @brief Parses the mime multipart request
 */
void HttpRequest::parseMultiPartFile() {
    m_tempFile.seek(0);
    bool finished=false;
    while (!m_tempFile.atEnd() && !finished && !m_tempFile.error()) {

        QString fieldName;
        QString fileName;
        QString contentType;
        while (!m_tempFile.atEnd() && !finished && !m_tempFile.error()) {
            QString line = QString::fromUtf8(m_tempFile.readLine(65536).trimmed());
            if (line.startsWith("--"+m_boundary)) { continue; }
            if (line.isEmpty()) { break; }
            if (line.startsWith("Content-Disposition:", Qt::CaseInsensitive)) { 
                if (line.contains("form-data", Qt::CaseInsensitive)) {
                    int start=line.indexOf(" name=\"", 0, Qt::CaseInsensitive);
                    int end=line.indexOf("\"",start+7);
                    if (start>=0 && end>=start) {
                        fieldName=line.mid(start+7,end-start-7);
                        }
                    start=line.indexOf(" filename=\"", 0, Qt::CaseInsensitive);
                    end=line.indexOf("\"",start+11);
                    if (start>=0 && end>=start) {
                        fileName=line.mid(start+11,end-start-11);
                        }
                    continue;
                    }
                }
            if (line.startsWith("Content-Type:", Qt::CaseInsensitive)) { 
                contentType = line.remove(QRegExp("^Content-Type:\\s*", Qt::CaseInsensitive));
                continue;
                }

            qDebug() << "HttpRequest: ignoring unsupported content part" << line;
            }

        QTemporaryFile* uploadedFile=0;
        QByteArray fieldValue;
        while (!m_tempFile.atEnd() && !finished && !m_tempFile.error()) {
            QByteArray line = m_tempFile.readLine(65536);
            if (line.startsWith("--"+m_boundary)) {
                // Boundary found. Until now we have collected 2 bytes too much,
                // so remove them from the last result
                if (fileName.isEmpty() && !fieldName.isEmpty()) {
                    // last field was a form field
                    fieldValue.remove(fieldValue.size()-2,2);
                    m_parameters.insert(fieldName,fieldValue);
                    }
                else if (!fileName.isEmpty() && !fieldName.isEmpty()) {
                    // last field was a file
                    uploadedFile->resize(uploadedFile->size()-2);
                    uploadedFile->flush();
                    uploadedFile->seek(0);
                    m_parameters.insert(fieldName,fileName);
                    m_uploadedFiles.insert(fieldName,uploadedFile);
                    m_contentTypes.insert(fieldName,contentType);
                    }
                if (line.contains(m_boundary+"--")) {
                    finished=true;
                    }
                break;
              } else {
                if (fileName.isEmpty() && !fieldName.isEmpty()) {
                    // this is a form field.
                    m_currentSize+=line.size();
                    fieldValue.append(line);
                    }
                else if (!fileName.isEmpty() && !fieldName.isEmpty()) {
                    // this is a file
                    if (!uploadedFile) {
                        uploadedFile=new QTemporaryFile();
                        uploadedFile->open();
                        }
                    uploadedFile->write(line);
                    if (uploadedFile->error()) {
                        qCritical("HttpRequest: error writing temp file, %s",qPrintable(uploadedFile->errorString()));
                        }
                    }
                }
            }
        }
    if (m_tempFile.error()) {
        qCritical("HttpRequest: cannot read temp file, %s",qPrintable(m_tempFile.errorString()));
        }
}


QString HttpRequest::header(const QString& name) const { 
    QMapIterator<QString, QString>  iterator(m_headers);
    while (iterator.hasNext()) {
        iterator.next();
        if (iterator.key().toLower() == name.toLower()) {
            return iterator.value();
            }
        }
    return QString();
}


QList<QString>  HttpRequest::headers(const QString& name) const {
    QMapIterator<QString, QString>  iterator(m_headers);
    while (iterator.hasNext()) {
        iterator.next();
        if (iterator.key().toLower() == name.toLower()) {
            return m_headers.values(iterator.key());
            }
        }
    return QList<QString>();
}

QString HttpRequest::statusString() const {
    QString text;
    switch (m_status) {
        case StatusWaitForRequest: text="Wait for Request"; break;
        case StatusWaitForHeader: text="Wait for Header"; break;
        case StatusWaitForBody: text="Wait for Body"; break;
        case StatusComplete: text="Complete"; break;
        case StatusAbort: text="Abort"; break;
        }
    return text;
}

