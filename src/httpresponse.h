/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpResponse_H_
#define _HttpResponse_H_

#include <QMap>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>
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
class HttpResponse : public QObject {
    Q_OBJECT
  public:
   ~HttpResponse();

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
    const QMap<QString, HttpCookie>& cookies() const { return m_cookies; };

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
     * @brief Writes last part of the response and closes the socket when possible
     *
     * When sending chunked response the last chunk is written (zero length), in
     * other case the socket is closed.
     *
     * Closing socket destroys the HttpConnection and HttpResponse classes.
     */
    void flush();


    /**
     * @brief Sets a cookie
     */
    void setCookie(const HttpCookie& cookie);

    /**
     * @brief Returns true if headers contains specific header
     */
    bool containsHeader(const QString& name) { return m_headers.contains(name); }


    /**
     * @brief Returns true if the http connection is in connected state
     */
    bool isConnected() const;

    void flushAndClose();

    void flushAndDelete();

    /**
     * @brief Closes socket and destroys connection. Should by called only when "chunked" transport is choosen
     *
     * Call flush() befor the close;
     */
    void close();

  private slots:
    void    slotWrite();

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    QTcpSocket *m_socket;
    HttpConnection *m_connection;

    QMap<QString, QString>      m_headers;
    QMap<QString, HttpCookie>   m_cookies;

    int m_statusCode;
    QString m_statusText;

    bool    m_sentHeaders;

    void    writeToSocket(const QByteArray& data); /// blocks!!! ??
    void    writeHeaders();

    QTimer     *m_writerTimer;

    QByteArray  m_dataBody;
    QByteArray  m_dataHeaders;
    int         m_dataBodyPointer;
    int         m_dataHeadersPointer;
    bool        m_canWriteToSocket;
    bool        m_closeAfterFlush;
    bool        m_deleteAfterFlush;
    bool        m_flushed;
    #endif
};

}

#endif
