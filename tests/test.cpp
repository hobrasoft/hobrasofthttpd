/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "test.h"
#include "httpsettings.h"
#include "httpserver.h"
#include "httptcpserver.h"
#include "httpconnection.h"
#include "httprequest.h"
#include "httpresponse.h"

#include <QDebug>
#include <QTcpSocket>
#include <QSignalSpy>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QList>
#include <QUrl>
#include <QEventLoop>

using namespace HobrasoftHttpd;

Test::Test() : QObject() {
}


void Test::initTestCase() {
    QSettings s;
    s.clear();
}


void Test::settingsDefaultValues() {
    HttpSettings s(0);
    QVERIFY(s.port()                    == 8080);
    QVERIFY(s.address()                 == QHostAddress::AnyIPv6);
    QVERIFY(s.timeout()                 == 600);
    QVERIFY(s.maxAge()                  == 3600);
    QVERIFY(s.encoding()                == "UTF-8");
    QVERIFY(s.docroot()                 == ".");
    QVERIFY(s.maxCachedFileSize()       == 1000000);
    QVERIFY(s.cacheTimeout()            == 600000);
    QVERIFY(s.indexFile()               == "index.html");
    QVERIFY(s.cacheOn()                 == false);
    QVERIFY(s.sessionCookieName()       == "sessionid");
    QVERIFY(s.sessionExpirationTime()   == 3600);
    QVERIFY(s.maxRequestSize()          == 16384);
    QVERIFY(s.maxMultiPartSize()        == 16728064);
    QVERIFY(s.useSSL()                  == false);
}


void Test::settingsCustomValues1() {
    QSettings q;
    q.setValue("http/port",                    8081); 
    q.setValue("http/address",                 "192.168.1.1");
    q.setValue("http/timeout",                 601);
    q.setValue("http/maxAge",                  3601);
    q.setValue("http/encoding",                "iso-8859-2");
    q.setValue("http/root",                    "/tmp");
    q.setValue("http/maxCachedFileSize",       1000001);
    q.setValue("http/cacheTimeout",            600001);
    q.setValue("http/indexFile",               "index.php");
    q.setValue("http/cacheOn",                 true);
    q.setValue("http/sessionCookieName",       "otherid");
    q.setValue("http/sessionExpirationTime",   3601);
    q.setValue("http/maxRequestSize",          16385);
    q.setValue("http/maxMultiPartSize",        16728065);
    q.setValue("http/useSSL",                  true);
    q.sync();
    HttpSettings s(&q,0);
    QVERIFY(s.port()                    == 8081);
    QVERIFY(s.address()                 == QHostAddress("192.168.1.1"));
    QVERIFY(s.timeout()                 == 601);
    QVERIFY(s.maxAge()                  == 3601);
    QVERIFY(s.encoding()                == "iso-8859-2");
    QVERIFY(s.docroot()                 == "/tmp");
    QVERIFY(s.maxCachedFileSize()       == 1000001);
    QVERIFY(s.cacheTimeout()            == 600001);
    QVERIFY(s.indexFile()               == "index.php");
    QVERIFY(s.cacheOn()                 == true);
    QVERIFY(s.sessionCookieName()       == "otherid");
    QVERIFY(s.sessionExpirationTime()   == 3601);
    QVERIFY(s.maxRequestSize()          == 16385);
    QVERIFY(s.maxMultiPartSize()        == 16728065);
    QVERIFY(s.useSSL()                  == true);
}


HobrasoftHttpd::HttpServer *Test::createServer(QObject *parent, const QHostAddress& address) {
    QSettings q;
    q.clear();
    q.setValue("http/address", address.toString());
    q.setValue("http/port",    8080); 
    q.setValue("http/root",    "."); 
    q.sync();
    HttpSettings *hq = new HttpSettings(&q,parent);
    HttpServer    *s = new HttpServer(hq,parent);
    return s;
}

void Test::httpserverStartIP(const QHostAddress& address) {
    QObject qobject(0);
    createServer(&qobject,address);

    QTcpSocket socket(this);
    socket.connectToHost(QHostAddress(address), 8080);
    QVERIFY(socket.waitForConnected(1000));
    socket.close();
}


void Test::httpserverStartIPv4() {
    httpserverStartIP(QHostAddress::LocalHost);
}


void Test::httpserverStartIPv6() {
    httpserverStartIP(QHostAddress::LocalHostIPv6);
}


void Test::httpserverHttpService() {
    QObject qobject(0);
    createServer(&qobject);

    QFile file("./hobrasofthttpd.test.html");
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    QNetworkAccessManager manager(this);
    QNetworkRequest rq(QUrl("http://127.0.0.1:8080/hobrasofthttpd.test.html"));
    QNetworkReply *reply = manager.get(rq);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll()  == "test");
    file.remove();

}


void Test::httpserverHttpService404() {
    QObject qobject(0);
    createServer(&qobject);

    QFile file("./hobrasofthttpd.test.html");
    file.remove();

    QNetworkAccessManager manager(this);
    QNetworkRequest rq(QUrl("http://127.0.0.1:8080/hobrasofthttpd.test.html"));
    QNetworkReply *reply = manager.get(rq);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::ContentNotFoundError);

}


void Test::httpserverHttpServiceShtmlInclude() {
    QObject qobject(0);
    createServer(&qobject);

    QFile file("./hobrasofthttpd.test.shtml");
    file.open(QIODevice::WriteOnly);
    file.write("<!-- #include \"hobrasofthttpd.test.html\" -->");
    file.close();

    QFile filei("./hobrasofthttpd.test.html");
    filei.open(QIODevice::WriteOnly);
    filei.write("test");
    filei.close();

    QNetworkAccessManager manager(this);
    QNetworkRequest rq(QUrl("http://127.0.0.1:8080/hobrasofthttpd.test.shtml"));
    QNetworkReply *reply = manager.get(rq);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll()  == "test");
    file.remove();
    filei.remove();
}


Httpd::Httpd(const HobrasoftHttpd::HttpSettings *settings, QObject *parent) : HttpServer(settings, parent) {
}


HobrasoftHttpd::HttpRequestHandler *Httpd::requestHandler(HobrasoftHttpd::HttpConnection *connection) {
    RequestMapper *mapper = new RequestMapper(connection);
    connect(mapper, SIGNAL(sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,   SIGNAL(sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));
    return mapper;
}


Httpd *Httpd::create(QObject *parent, const QHostAddress& address) {
    QSettings q;
    q.clear();
    q.setValue("http/address", address.toString());
    q.setValue("http/port",    8080); 
    q.setValue("http/root",    "."); 
    q.sync();
    HttpSettings *hq = new HttpSettings(&q,parent);
    return new Httpd(hq,parent);
}


RequestMapper::RequestMapper(HobrasoftHttpd::HttpConnection *parent) : HobrasoftHttpd::HttpRequestHandler(parent) {
}


void RequestMapper::service(HttpRequest *request, HttpResponse *response) {
    QVERIFY(request  != NULL);
    QVERIFY(response != NULL);
    QVERIFY(request->status() == HttpRequest::StatusComplete);
    HttpConnection *connection = request->connection();
    QVERIFY(connection != NULL);
    emit sigService(request, response);
}


void Test::slotServiceGet(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->method() == "GET");
    QVERIFY(request->path() == "/hobrasofthttpd.test.html");
    QVERIFY(request->version() == "HTTP/1.1");
    QVERIFY(request->parameter("A") == "1");
    QVERIFY(request->parameter("B") == "2");
    QVERIFY(request->parameterMap().size() == 2);
    QVERIFY(request->parameters("A").size() == 1);
    QVERIFY(request->parameters("B").size() == 1);
    QVERIFY(request->parameters("A").first() == "1");
    QVERIFY(request->parameters("B").first() == "2");
    response->write("test");
    response->flush();
}


#define WAITFORREPLY() { \
    QEventLoop loop; \
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit())); \
    QTimer::singleShot(1000, &loop, SLOT(quit())); \
    loop.exec(); \
    QVERIFY(reply->isFinished()); \
    QVERIFY(reply->error() == QNetworkReply::NoError); \
    }

void Test::httpServiceGet() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService   (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceGet(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url("http://127.0.0.1:8080/hobrasofthttpd.test.html");
    url.addQueryItem("A", "1");
    url.addQueryItem("B", "2");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    QNetworkReply *reply = manager.get(rq);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test"); \

}


void Test::slotServicePost(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->method() == "POST");
    QVERIFY(request->path() == "/hobrasofthttpd.test.html");
    QVERIFY(request->version() == "HTTP/1.1");
    QVERIFY(request->header("Content-type") == "text/text");
    QVERIFY(request->headers("Content-type").contains("text/text") == true);
    QVERIFY(request->headerMap().values("Content-type").contains("text/text") == true);
    QVERIFY(request->body() == "abcd");
    QVERIFY(request->parameter("A") == "1");
    QVERIFY(request->parameter("B") == "2");
    QVERIFY(request->parameterMap().size() == 2);
    QVERIFY(request->parameters("A").size() == 1);
    QVERIFY(request->parameters("B").size() == 1);
    QVERIFY(request->parameters("A").first() == "1");
    QVERIFY(request->parameters("B").first() == "2");
    response->write("test");
    response->flush();
}


void Test::httpServicePost() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService    (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServicePost(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url("http://127.0.0.1:8080/hobrasofthttpd.test.html");
    url.addQueryItem("A", "1");
    url.addQueryItem("B", "2");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    rq.setRawHeader("Content-type", "text/text");
    QNetworkReply *reply = manager.post(rq, "abcd");
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test"); \

}


void Test::slotServiceCookie(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->method() == "GET");
    QVERIFY(request->path() == "/hobrasofthttpd.test.html");
    QVERIFY(request->version() == "HTTP/1.1");
    QVERIFY(request->cookie("kuki") == "xyz");
    response->write("test");
    response->flush();
}


void Test::httpServiceCookie() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService      (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceCookie(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url("http://127.0.0.1:8080/hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    QNetworkCookie c("kuki", "xyz");
    rq.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue<QNetworkCookie>(c));
    QNetworkReply *reply = manager.get(rq);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test"); \

}


void Test::requestUrlDecode() {
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("%25") == "%");
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("%2B") == "+");
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("A+B") == "A B");
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("A%2BB") == "A+B");
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("A%25B") == "A%B");
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("%") == "%");        // Nedefinované, ale je potřeba otestovat
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("%%") == "%%");      // Nedefinované, ale je potřeba otestovat
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("%%%") == "%%%");    // Nedefinované, ale je potřeba otestovat
}


void Test::slotServiceUpload(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->uploadedFile("file1") != NULL);
    QTemporaryFile *file1 = request->uploadedFile("file1");
    QVERIFY(file1 != NULL);
    QVERIFY(file1->readAll() == "ABCD");
    file1->close();
    file1->deleteLater();

    QVERIFY(request->uploadedFile("file2") != NULL);
    QTemporaryFile *file2 = request->uploadedFile("file2");
    QVERIFY(file2 != NULL);
    QVERIFY(file2->readAll() == "XYZ");
    file2->close();
    file2->deleteLater();

    QVERIFY(request->uploadedFile("file3") == NULL);

    response->write("test");
    response->flush();
}


void Test::httpServiceUpload() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService      (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceUpload(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QByteArray data;
    data += "--BOUNDARY\r\n";
    data += "Content-Disposition: form-data; name=\"file1\"; filename=\"file1.txt\"\r\n";
    data += "Content-Type: text/text\r\n\r\n";
    data += "ABCD\r\n";
    data += "--BOUNDARY\r\n";
    data += "Content-Disposition: form-data; name=\"file2\"; filename=\"file2.txt\"\r\n";
    data += "Content-Type: text/text\r\n\r\n";
    data += "XYZ\r\n";
    data += "--BOUNDARY--\r\n";

    QUrl url("http://127.0.0.1:8080/hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    rq.setRawHeader("Content-Type", "multipart/form-data; boundary=BOUNDARY");
    QNetworkReply *reply = manager.post(rq,data);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test"); \
}

