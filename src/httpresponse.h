/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpResponse_H_
#define _HttpResponse_H_

#include <QMap>
#include <QString>
#include <QTcpSocket>
#include "httpcookie.h"

namespace HobrasoftHttpd {
class HttpConnection;
class HttpCookie;

/**
 * @brief Response to HTTP request - headers, cookies and body
 *
 * Can send responses of chunked and regular type. 
 *
 * It is possible to send more responses to one request. In fact, this is the common practice
 * when using HTML5 event streams. 
 */
class HttpResponse {
  public:

    /**
     * @brief Constructor sets default values for headers (status 200, OK)
     */
    HttpResponse(HttpConnection *);

    /**
     * @brief Sets or rewrite one header
     */
    void setHeader(const QString& name, const QString& value);

    /**
     * @brief Sets or rewrite one header
     */
    void setHeader(const QString& name, int value);

    /**
     * @brief Sets the header sending
     *
     * When the response is of chunked type, the headers are sent only once even
     * if multiple responses are sent to one request. You can suppress the headers sending
     * for the request.
     *
     * @param send - when set the headers are send (default), otherwise the headers are not send in response
     */
    void setSendHeaders(bool send) { m_sentHeaders = !send; }

    /**
     * @brief Returns headers of the response in QMap
     */
    QMap<QString, QString>& headers();

    /**
     * @brief Clears all headers set
     */
    void clearHeaders();

    /**
     * @brief Returns cookies of the response in QMap
     */
    QMap<QString, HttpCookie>& cookies();

    /**
     * @brief Returns cookie
     */
    HttpCookie cookie(const QString& name) { return m_cookies.value(name); }

    /**
     * @brief Set the status code and the description of the response
     */
    void setStatus(int code, const QString& description = QString());

    /**
     * @brief Writes data to response body
     *
     * Important note to chunked transport: write complete response at once.
     * Every call of this method sends one chunk to the browser. You should 
     * write complete response to your own buffer and send the response in
     * one call of this method.
     */
    void write(const QByteArray& data);


    /**
     * @brief Flushed sockets data to network
     */
    void flushSocket();


    /**
     * @brief Closes socket and destroys connection
     */
    void close();


    /**
     * @brief Writes last part of the response and closes the socket when possible
     *
     * When sending chunked response the last chunk is written (zero length), in
     * other case the socket is closed.
     *
     * Closing socket destroys the HttpConnection and HttpResponse classes.
     */
    void flush();

    /**
     * @brief Returns true if last part of the response was written to socket
     */
    bool hasSentLastPart() const { return m_sentLastPart; }

    /**
     * @brief Sets a cookie
     */
    void setCookie(const HttpCookie& cookie);

    /**
     * @brief Returns true if headers contains specific header
     */
    bool containsHeader(const QString& name) { return m_headers.contains(name); }


    bool isConnected() const;

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    QTcpSocket *m_socket;
    HttpConnection *m_connection;

    QMap<QString, QString>      m_headers;
    QMap<QString, HttpCookie>   m_cookies;

    int m_statusCode;
    QString m_statusText;

    bool    m_sentHeaders;;
    bool    m_sentLastPart;

    void    writeToSocket(const QByteArray& data); /// blocks!!! ??
    void    writeHeaders();
    void    write(const QByteArray& data, bool lastPart);
    #endif
};

}

#endif
