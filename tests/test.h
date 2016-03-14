/**
 * @file
 * 
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _Test_H_
#define _Test_H_

#include <QObject>
#include <QTest>
#include <QtTest/QtTest>
#include <QHostAddress>
#include <QThread>
#include "httpserver.h"
#include "httprequesthandler.h"

namespace HobrasoftHttpd {
class HttpServer;
class HttpConnection;
class HttpHandler;
class HttpSettings;
class HttpRequest;
class HttpRequestHandler;
class HttpResponse;
class HttpSessionStore;
};


class RequestMapper : public HobrasoftHttpd::HttpRequestHandler {
    Q_OBJECT
  public:
    RequestMapper(HobrasoftHttpd::HttpConnection *parent);
    void service(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
  signals:
    void sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *);
};


class Httpd : public HobrasoftHttpd::HttpServer {
    Q_OBJECT
  public:
    Httpd(const HobrasoftHttpd::HttpSettings*, QObject *parent);
    static Httpd *create(QObject *parent, const QHostAddress& address = QHostAddress::LocalHost);
    virtual HobrasoftHttpd::HttpRequestHandler *requestHandler(HobrasoftHttpd::HttpConnection *);
  signals:
    void sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *);
};


class TestWorker : public QObject {
    Q_OBJECT
  public:
    TestWorker(const QHostAddress& host, const int& port) { 
        m_localhost = host;
        m_port = port; 
        }

  signals:
    void    finished();

  public slots:
    void    start();

  private:
    QHostAddress m_localhost;
    int          m_port;
};

/**
 * @brief
 */
class Test : public QObject {
    Q_OBJECT
  public:
    Test();

  private slots:
    void    initTestCase();

    void    settingsDefaultValues();
    void    settingsCustomValues1();
    void    requestUrlDecode();
    void    httpCookie();
    void    httpserverStartIPv4();
    void    httpserverStartIPv6();
    void    httpserverHttpService();
    void    httpserverHttpService404();
    void    httpserverHttpServiceShtmlInclude();
    void    httpServiceGet();
    void    httpServicePost();
    void    httpServiceCookie();
    void    httpServiceUpload();
    void    httpServiceUploadInsensitive();
    void    httpServiceResponse();
    void    httpServiceResponseChunked();
    void    httpServiceSession();
    void    httpServiceAbort();

    void    slotServiceGet              (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServicePost             (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceCookie           (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceUpload           (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceUploadInsensitive(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceResponse         (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceResponseChunked  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceSession          (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceAbort            (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

  private:
    void    httpserverStartIP(const QHostAddress&);
    HobrasoftHttpd::HttpServer *createServer(QObject *parent, const QHostAddress& address = QHostAddress::LocalHost);
    HobrasoftHttpd::HttpSessionStore *m_sessionStore;

    QHostAddress m_localhost;
    bool    m_ipv6;
    int     m_port;
    QThread m_workerThread;
};

#endif
