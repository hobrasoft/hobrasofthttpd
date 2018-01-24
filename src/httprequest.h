/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpRequest_H_
#define _HttpRequest_H_

#include <QTemporaryFile>
#include <QDateTime>
#include <QTcpSocket>
#include <QString>
#include "httpconnection.h"

namespace HobrasoftHttpd {


/**
 * @brief Processes HTTP request, parses headers, body and files sent by HTTP protocol
 */
class HttpRequest {
  public:
    /**
     * @brief Inner state of request, changes when parts of request are read from networkd
     */
    enum Status { 
        StatusWaitForRequest,       ///< Waits for request
        StatusWaitForHeader,        ///< Waits for header
        StatusWaitForBody,          ///< Waits for body
        StatusComplete,             ///< Request is complete
        StatusAbort                 ///< Request is canceled
        };

    /**
     * @brief Constructor sets default falues from configuration
     */
    HttpRequest(HttpConnection *connection);

    /**
     * @brief Returns HttpConnection of the request
     */
    HttpConnection *connection() const { return m_connection; }


    /**
     * @brief Returns peer's host address
     */
    QHostAddress peerAddress() const { return m_connection->peerAddress(); }


    /**
     * @brief Reads data from socket.
     *
     * Called from HttpConnection when reading the request
     */
    void readFromSocket(QTcpSocket *socket);

    /**
     * @brief Returns current status of the request
     */
    Status status() const { return m_status; }

    /**
     * @brief Returns current status of the request as a string
     */
    QString statusString() const;

    /**
     * @brief Returns used HTTP method of the request (GET, PUT, POST or DELETE)
     */
    QString    method() const { return m_method; }

    /**
     * @brief Returns path of the request (/files/index.html)
     */
    QString    path() const { return urlDecode(m_path); }

    /**
     * @brief Returns full path of the request (/files/index.html) including parameters
     */
    QString    fullPath() const { return urlDecode(m_fullpath); }

    /**
     * @brief Returns version of HTTP protocol (HTTP/1.1)
     */
    QString    version() const { return m_version; }

    /**
     * @brief Returns requested header value, case insensitive
     */
    QString    header(const QString& name) const;

    /**
     * @brief Returns all headers of HTTP request in QList, case insensitive
     *
     * Cookies are not contined in the returnetd values. If you want to read cookies, use cookie() method instead.
     */
    QList<QString>  headers(const QString& name) const;

    /**
     * @brief Returns all headers of HTTP request in QMap
     *
     * Cookies are not contined in the returnetd values. If you want to read cookies, use cookie() method instead.
     */
    QMultiMap<QString, QString>     headerMap() const { return m_headers; }

    /**
     * @brief Returns parameter of the HTTP request
     *
     * Parameters are typically used in GET requests:
     *
     * http://my-own-server.com/path?param1=abc&param2=xyz
     */
    QString  parameter(const QString& name) const { return m_parameters.value(name); }

    /**
     * @brief Returns all parameters of the HTTP request in QList
     */
    QList<QString>  parameters(const QString& name) const {  return m_parameters.values(name); }

    /**
     * @brief Returns all parameters of the HTTP request in QMap
     */
    QMultiMap<QString, QString>     parameterMap() const { return m_parameters; }

    /**
     * @brief Returns the body of the request
     *
     * Use this method when you need to read POST requests
     */
    QByteArray  body() const { return m_bodyData; }

    /**
     * @brief Converts URL encoded string to UTF8 string
     */
    static QString urlDecode(const QString& source); 

    /**
     * @brief Returns cookie
     */
    QString cookie(const QString& name) const { return m_cookies.value(name); }

    /**
     * @brief Returns all cookies of the request in QMap
     */
    const QMap<QString, QString>& cookieMap() { return m_cookies; }

    /**
     * @brief Returns temporary file with uploaded file from html form
     *
     * If the returned file is closed, then the file is deleted.
     * If you want to reuse the file, do not close it.
     */
    QTemporaryFile *uploadedFile(const QString& fieldName) { return m_uploadedFiles.value(fieldName); }

    /**
     * @brief Returns list of uploaded files
     *
     * Returned map:
     * - Key: name of the field
     * - Value: opened temporary file
     *
     * If the returned file is closed, then the file is deleted.
     * If you want to reuse the file, do not close it.
     */
    const QMap<QString, QTemporaryFile *>&  uploadedFiles() const { return m_uploadedFiles; }


    /**
     * @brief Returns list of content types of uploaded files
     * 
     * Returned map:
     * - Key: name of the fiels
     * - Value: content type
     */
    const QMap<QString, QString>& contentTypes() const { return m_contentTypes; }


    /**
     * @brief Returns content type of uploaded file
     */
    QString contentType(const QString& fieldName) const { return m_contentTypes.value(fieldName); }


    /**
     * @brief Returns date and time of the request
     */
    const QDateTime& datetime() const { return m_datetime; }


  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    QMultiMap<QString, QString>         m_headers;
    QMultiMap<QString, QString>         m_parameters;
    QMap<QString, QTemporaryFile *>     m_uploadedFiles;
    QMap<QString, QString>              m_contentTypes;
    QMap<QString, QString>              m_cookies;
    QByteArray                          m_bodyData;
    QString                             m_method;
    QString                             m_path;
    QString                             m_fullpath;
    QString                             m_version;
    Status                              m_status;
    QByteArray                          m_boundary;
    int                                 m_currentSize;
    int                                 m_expectedBodySize;
    QString                             m_currentHeader;
    QTemporaryFile                      m_tempFile;
    HttpConnection                     *m_connection;
    QDateTime                           m_datetime;
    #endif

    void    parseMultiPartFile();

    void    readRequest(QTcpSocket* socket);

    void    readHeader (QTcpSocket* socket);

    void    readBody   (QTcpSocket* socket);

    void    decodeRequestParams();

    void    extractCookies();

};

}

#endif
