/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpConnection_H_
#define _HttpConnection_H_

#include <QObject>
#include <QTcpSocket>
#include <QSslCertificate>
#include <QTimer>

namespace HobrasoftHttpd {

class HttpRequest;
class HttpResponse;
class HttpRequestHandler;
class HttpServer;
class HttpSettings;


/**
 * @brief One single connection to http server
 */
class HttpConnection : public QObject {
    Q_OBJECT
  public:

   ~HttpConnection();

    /**
     * @brief Constructor is called automatically when new request arrived
     */
    HttpConnection(HttpServer *parent, QTcpSocket *socket);

    /**
     * @brief Returns new instance of class HttpResponse
     *
     * The returned class is connected with opened socket. Method is used in 
     * specialized handlers derived from HttpRequestHandler where it is demanded
     * to make multiple responses to one request. This typically happened in
     * HTML5 event streams. Derived class should call this method indirectly using 
     * HttpRequestHandler::response().
     *
     * Timeout timer is restarted when this method is called.
     *
     * @see HttpRequestHandler::response()
     *
     */
    HttpResponse response();

    /**
     * @brief Returns pointer to settings used in the class
     *
     * @see HttpSettings
     */
    const HttpSettings *settings() const;

    /**
     * @brief Returns pointer to http server
     *
     * @see HttpServer
     */
    HttpServer *httpServer() const { return m_parent; }

    /**
     * @brief Returns the socket
     */
    QTcpSocket *socket() const { return m_socket; }

    /**
     * @brief Returns true if the client is verifiend using SSL
     */
    bool verified() const;

    /**
     * @brief  
     */
    void setPeerCertificate(const QSslCertificate&);

    /**
     * @brief Returns CommonName field from certificate
     */
    QString commonName() const;

    /**
     * @brief Returns Peer's certificate
     */
    const QSslCertificate peerCertificate() const { return m_peerCertificate; }

    bool isConnected() const { return m_connected; }


  public slots:
    /**
     * @brief Closes the connection.
     *
     * When the connection is closed, all descendant objects are destroyed.
     */
    void close();

  private slots:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    void        slotTimeout();
    void        slotRead();
    void        slotDisconnected();

  private:
    QTcpSocket          *m_socket;
    QTimer              *m_timeout;
    HttpRequest         *m_request;
    HttpRequestHandler  *m_handler;
    HttpServer          *m_parent;
    QSslCertificate      m_peerCertificate;
    bool                 m_connected;
    bool                 m_inService;
    #endif
    
};

}

#endif
