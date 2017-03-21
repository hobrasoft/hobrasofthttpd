/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpServer_H_
#define _HttpServer_H_

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>
#include <QString>
#include <QSslError>
#include <QSet>
#include <QPointer>
#include "testsettings.h"

namespace HobrasoftHttpd {

class HttpRequestHandler;
class HttpConnection;
class HttpSettings;
class HttpTcpServer;

/**
@brief General single-threaded, event-driven HTTP server 

Static content server
---------------------
The class can be used as a static content server without extending. The .html and .shtml
formats are recognized. Html files are returned without any change. In shtml file all
occurences of #include pragma is found and replaced with requested file. Only one pragma
on line is possible. No other strings are allowed on line with #include pragma:

@code
<!-- #include "file.html" -->
@endcode

Extending server
----------------
If you want to use some dynamic content you have to reimplement the Http::requestHandler() method.
The method is called for every request and it must return new instance of your own HttpRequestHandler derived class:

@code 
HttpRequestHandler *Httpd::requestHandler(HttpConnection *connection) {
    return new MyOwnRequestHandler(connection);
}
@endcode 


The class MyOwnRequestHandler() is derived from HttpRequestHandler where the HttpRequestHandler::service() is reimplemented:


@code
void RequestMapper::service(HttpRequest *request, HttpResponse *response) {
    QString path = request->path();

    #define ROUTER(address, hclass) \
        if (path.startsWith(address)) { \
            HttpRequestHandler *controller = new hclass (connection()); \
            controller->service(request, response); \
            return; \
            }

    ROUTER("/translations",     ControllerTranslations);
    ROUTER("/errorevents",      ControllerErrorEvents);
    ROUTER("/callqueue",        ControllerCallQueue);

    HttpRequestHandler::service(request, response);
    response->flush();
}
@endcode

Configuration
-------------
HttpServer class has two constructors:
- HttpServer::HttpServer(QObject *parent)
- HttpServer::HttpServer(const HttpSettings* settings, QObject *parent)

When the first constructor is used then the default QSettings is used to read the configuration.
In the second constructor you can use your own instance of HttpSettings class or your own derived class.
Using different configuration classes you can start multiple instances of HttpServer in your application
each listening on different addresses or ports.

*/
class HttpServer : public QObject {
    FRIEND_CLASS_TEST;
    Q_OBJECT
  public:

    /**
     * @brief Constructor using default HttpSettings object
     */
    HttpServer(QObject *parent);

    /**
     * @brief Constructor using your own HttpSettings
     *
     * When using different settings it is possible to start multiple instances of the HttpServer.
     */
    HttpServer(const HttpSettings* settings, QObject *parent);

    /**
     * @brief Starts of restart HttpServer with new parameters
     */
    void            start();

    /**
     * @brief Creates new request handler and returs pointer to it
     *
     * This method should be reimplemented in derived classes
     */
    virtual HttpRequestHandler *requestHandler(HttpConnection *);

    /**
     * @brief Returs pointer to HttpSettings used in the HttpServer
     */
    const HttpSettings *settings() const { return m_settings; }

    QVariant webStatus() const;

    QList<QPointer<HobrasoftHttpd::HttpConnection> >   connections() const { return m_connections; }

  protected:

    /**
     * @brief Closes the QTcpServer bind with your HttpServer
     */
    void  close();

  private slots:

    /**
     * @brief Slot is invoked when QTcpServer::newConnection() signal arrived.
     *
     * The method processes new request.
     */
    void            slotNewConnection();


    void            slotConnectionClosed(QObject *);

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    HttpTcpServer       *m_server;
    const HttpSettings  *m_settings;
    QList<QPointer<HobrasoftHttpd::HttpConnection> > m_connections;
    #endif

};
    
}

#endif
