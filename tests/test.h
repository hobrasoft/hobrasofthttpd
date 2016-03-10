/**
 * @file
 * 
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _Test_H_
#define _Test_H_

#include <QObject>
#include <QtTest/QtTest>
#include <QHostAddress>
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
    void    httpserverStartIPv4();
    void    httpserverStartIPv6();
    void    httpserverHttpService();
    void    httpserverHttpService404();
    void    httpserverHttpServiceShtmlInclude();
    void    httpServiceGet();
    void    httpServicePost();
    void    httpServiceCookie();
    void    requestUrlDecode();

    void    slotServiceGet   (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServicePost  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    void    slotServiceCookie(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

  private:
    void    httpserverStartIP(const QHostAddress&);
    HobrasoftHttpd::HttpServer *createServer(QObject *parent, const QHostAddress& address = QHostAddress::LocalHost);
};

#endif

