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
#include "httpsessionstore.h"
#include "httpgzipcompression.h"
#include "staticfilecontroller.h"

#include <QDebug>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QSignalSpy>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QList>
#include <QUrl>
#include <QEventLoop>
#include <QSslConfiguration>
#include <QSslKey>
#if QT_VERSION > 0x050000
#include <QUrlQuery>
#endif

#define QURL(x) \
    QUrl(QString("http://%1:%2/%3") \
        .arg(m_localhost.toString()) \
        .arg(m_port) \
        .arg(x))

#define QURLSSL(x) \
    QUrl(QString("https://localhost:%1/%2") \
        .arg(m_port) \
        .arg(x))

using namespace HobrasoftHttpd;

Test::Test() : QObject() {
}


void Test::initTestCase() {
    QSettings s;
    s.clear();

    m_localhost = QHostAddress::LocalHost;
    m_port = 0;
    m_ipv6 = false;
    QUdpSocket socket4(this);
    for (int i=8080; i<9000; i++) {
        if (socket4.bind(QHostAddress::LocalHost, i)) {
            m_port = i;
            break;
            }
        }

    QUdpSocket socket6(this);
    if (socket6.bind(QHostAddress::LocalHostIPv6, m_port)) {
        m_ipv6 = true;
        // m_localhost = QHostAddress::LocalHostIPv6;
        }

}


void Test::cleanupTestCase() {
    QFile::remove("./hobrasofthttpd.test.shtml");
    QFile::remove("./hobrasofthttpd.test.html");
}


void Test::settingsDefaultValues() {
    HttpSettings s(0);
    QVERIFY(s.port()                    == 8080);
    QVERIFY(s.address()                 == QHostAddress::AnyIPv6 ||
            s.address()                 == QHostAddress::Any);
    QVERIFY(s.timeout()                 == 600);
    QVERIFY(s.maxAge()                  == 3600);
    QVERIFY(s.encoding()                == "UTF-8");
    QVERIFY(s.docroot()                 == ".");
    QVERIFY(s.indexFile()               == "index.html");
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
    q.setValue("http/indexFile",               "index.php");
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
    QVERIFY(s.indexFile()               == "index.php");
    QVERIFY(s.sessionCookieName()       == "otherid");
    QVERIFY(s.sessionExpirationTime()   == 3601);
    QVERIFY(s.maxRequestSize()          == 16385);
    QVERIFY(s.maxMultiPartSize()        == 16728065);
    QVERIFY(s.useSSL()                  == true);
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
    // UTF-8
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode("%C5%BD") == QString::fromUtf8("Ž"));
    QVERIFY(HobrasoftHttpd::HttpRequest::urlDecode(
       "p%C5%99%C3%ADli%C5%A1%20%C5%BElu%C5%A5ou%C4%8Dk%C3%BD%20k%C5%AF%C5%88%20%C3%BAp%C4%9Bl%20%C4%8F%C3%A1belsk%C3%A9%20%C3%B3dy" 
       ) == QString::fromUtf8("příliš žluťoučký kůň úpěl ďábelské ódy"));

}


void Test::httpCookie() {
    HttpCookie c("name", "value", 100);
    QVERIFY(c.m_name    == "name");
    QVERIFY(c.m_value   == "value");
    QVERIFY(c.m_maxAge  == 100);
    QVERIFY(c.m_path    == "/");
    QVERIFY(c.m_comment == "");
    QVERIFY(c.m_domain  == "");
    QVERIFY(c.m_secure  == false); 
    QVERIFY(c.name()    == "name");
    QVERIFY(c.value()   == "value");
    QVERIFY(c.maxAge()  == 100);
    QVERIFY(c.path()    == "/");
    QVERIFY(c.comment() == "");
    QVERIFY(c.domain()  == "");
    QVERIFY(c.secure()  == false); 
    QVERIFY(c.toByteArray() == "name=value; Max-Age=100; Path=/; Version=1");

    HttpCookie c1("name", "value", 100, "/path");
    QVERIFY(c1.m_name    == "name");
    QVERIFY(c1.m_value   == "value");
    QVERIFY(c1.m_maxAge  == 100);
    QVERIFY(c1.m_path    == "/path");
    QVERIFY(c1.m_comment == "");
    QVERIFY(c1.m_domain  == "");
    QVERIFY(c1.m_secure  == false); 
    QVERIFY(c1.name()    == "name");
    QVERIFY(c1.value()   == "value");
    QVERIFY(c1.maxAge()  == 100);
    QVERIFY(c1.path()    == "/path");
    QVERIFY(c1.comment() == "");
    QVERIFY(c1.domain()  == "");
    QVERIFY(c1.secure()  == false); 
    QVERIFY(c1.version() == 1); 
    QVERIFY(c1.toByteArray() == "name=value; Max-Age=100; Path=/path; Version=1");

    HttpCookie c2("name", "value", 100, "/path", "comment");
    QVERIFY(c2.m_name    == "name");
    QVERIFY(c2.m_value   == "value");
    QVERIFY(c2.m_maxAge  == 100);
    QVERIFY(c2.m_path    == "/path");
    QVERIFY(c2.m_comment == "comment");
    QVERIFY(c2.m_domain  == "");
    QVERIFY(c2.m_secure  == false); 
    QVERIFY(c2.name()    == "name");
    QVERIFY(c2.value()   == "value");
    QVERIFY(c2.maxAge()  == 100);
    QVERIFY(c2.path()    == "/path");
    QVERIFY(c2.comment() == "comment");
    QVERIFY(c2.domain()  == "");
    QVERIFY(c2.secure()  == false); 
    QVERIFY(c2.version() == 1); 
    QVERIFY(c2.toByteArray() == "name=value; Comment=comment; Max-Age=100; Path=/path; Version=1");

    HttpCookie c3("name", "value", 100, "/path", "comment", "hobrasoft.cz");
    QVERIFY(c3.m_name    == "name");
    QVERIFY(c3.m_value   == "value");
    QVERIFY(c3.m_maxAge  == 100);
    QVERIFY(c3.m_path    == "/path");
    QVERIFY(c3.m_comment == "comment");
    QVERIFY(c3.m_domain  == "hobrasoft.cz");
    QVERIFY(c3.m_secure  == false); 
    QVERIFY(c3.name()    == "name");
    QVERIFY(c3.value()   == "value");
    QVERIFY(c3.maxAge()  == 100);
    QVERIFY(c3.path()    == "/path");
    QVERIFY(c3.comment() == "comment");
    QVERIFY(c3.domain()  == "hobrasoft.cz");
    QVERIFY(c3.secure()  == false); 
    QVERIFY(c3.version() == 1); 
    QVERIFY(c3.toByteArray() == "name=value; Comment=comment; Domain=hobrasoft.cz; Max-Age=100; Path=/path; Version=1");

    HttpCookie c4("name", "value", 100, "/path", "comment", "hobrasoft.cz", true);
    QVERIFY(c4.m_name    == "name");
    QVERIFY(c4.m_value   == "value");
    QVERIFY(c4.m_maxAge  == 100);
    QVERIFY(c4.m_path    == "/path");
    QVERIFY(c4.m_comment == "comment");
    QVERIFY(c4.m_domain  == "hobrasoft.cz");
    QVERIFY(c4.m_secure  == true); 
    QVERIFY(c4.name()    == "name");
    QVERIFY(c4.value()   == "value");
    QVERIFY(c4.maxAge()  == 100);
    QVERIFY(c4.path()    == "/path");
    QVERIFY(c4.comment() == "comment");
    QVERIFY(c4.domain()  == "hobrasoft.cz");
    QVERIFY(c4.secure()  == true); 
    QVERIFY(c4.version() == 1); 
    QVERIFY(c4.toByteArray() == "name=value; Comment=comment; Domain=hobrasoft.cz; Max-Age=100; Path=/path; Secure; Version=1");

    c4.setName("a1");
    QVERIFY(c4.name() == "a1");
    c4.setValue("a2");
    QVERIFY(c4.value() == "a2");
    c4.setComment("a3");
    QVERIFY(c4.comment() == "a3");
    c4.setDomain("a4");
    QVERIFY(c4.domain() == "a4");
    c4.setMaxAge(19);
    QVERIFY(c4.maxAge() == 19);
    c4.setPath("a5");
    QVERIFY(c4.path() == "a5");
    c4.setSecure(true);
    QVERIFY(c4.secure() == true);
    c4.setSecure(false);
    QVERIFY(c4.secure() == false);
    c4.setVersion(2);
    QVERIFY(c4.version() == 2);

}


HobrasoftHttpd::HttpServer *Test::createServer(QObject *parent, const QHostAddress& address, bool gzip) {
    QSettings q;
    q.clear();
    q.setValue("http/address", address.toString());
    q.setValue("http/port",    m_port); 
    q.setValue("http/root",    "."); 
    q.setValue("http/threads", false);
    q.setValue("http/gzip",    gzip);
    q.sync();
    HttpSettings *hq = new HttpSettings(&q,parent);
    HttpServer    *s = new HttpServer(hq,parent);
    return s;
}

void Test::httpserverStartIP(const QHostAddress& address) {
    QObject qobject(0);
    createServer(&qobject,address);

    QTcpSocket socket(this);
    socket.connectToHost(QHostAddress(address), m_port);
    QVERIFY(socket.waitForConnected(1000));
    socket.close();
}


void Test::httpserverStartIPv4() {
    httpserverStartIP(QHostAddress::LocalHost);
}


void Test::httpserverStartIPv6() {
    if (!m_ipv6) {
        QSKIP("IPv6 support missing", SkipAll);
        }
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
    QNetworkRequest rq(QURL("hobrasofthttpd.test.html"));
    QNetworkReply *reply = manager.get(rq);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll() == "test");
    file.remove();

}


void Test::crc32() {
    quint32 crc;
    QString crcstring;

    crc = HttpGZipCompression::crc32(QByteArray("ABCD", 4));
    crcstring = QString("0x%1").arg(crc, 8, 16, QLatin1Char('0')).toUpper();
    QVERIFY(crcstring == "0XDB1720A5");

    crc = HttpGZipCompression::crc32(QByteArray("1234567890", 10));
    crcstring = QString("0x%1").arg(crc, 8, 16, QLatin1Char('0')).toUpper();
    QVERIFY(crcstring == "0X261DAEE5");

    QByteArray body("AAAAABBBBBCCCCCDDDDDEEEEEFFFFFGGGGGHHHHHIIIIIKKKKK"
                    "AAAAABBBBBCCCCCDDDDDEEEEEFFFFFGGGGGHHHHHIIIIIKKKKK", 100);
    crc = HttpGZipCompression::crc32(body);
    crcstring = QString("0x%1").arg(crc, 8, 16, QLatin1Char('0')).toUpper();
    QVERIFY(crcstring == "0X178D6B76");

}


void Test::gzip() {
    QByteArray data("ABCD01234567890", 14);
    QVERIFY(HttpGZipCompression::compressData(data).toHex() == 
            "1f8b080000000000000b737472763130343236313533b7b004007debbeb40e000000");
}
    


void Test::httpserverHttpServiceGzip() {
    QObject qobject(0);
    createServer(&qobject, QHostAddress::LocalHost, true);

    QByteArray text = "test test test test test test test test test test test test test test test test test test test \n";
    QFile file("./hobrasofthttpd.test.html");
    file.open(QIODevice::WriteOnly);
    file.write(text);
    file.close();

    QNetworkAccessManager manager(this);
    QNetworkRequest rq(QURL("hobrasofthttpd.test.html"));
    QNetworkReply *reply = manager.get(rq);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll() == text);
    file.remove();

}



void Test::httpserverHttpService404() {
    QObject qobject(0);
    createServer(&qobject);

    QFile file("./hobrasofthttpd.test404.html");
    file.remove();

    QNetworkAccessManager manager(this);
    QNetworkRequest rq(QURL("hobrasofthttpd.test404.html"));
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
    QNetworkReply *reply;
    reply = manager.get(QNetworkRequest(QURL("hobrasofthttpd.test.shtml")));
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll()  == "test");
    file.remove();
    filei.remove();

    // Test rekurze include
    QFile file2("./hobrasofthttpd.test.shtml");
    file2.open(QIODevice::WriteOnly);
    file2.write("<!-- #include \"hobrasofthttpd.test.shtml\" -->");
    file2.close();

    reply = manager.get(QNetworkRequest(QURL("hobrasofthttpd.test.shtml")));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    #if QT_VERSION > 0x050000
    QVERIFY(reply->error() == QNetworkReply::InternalServerError);
    #endif
    file2.remove();

}


Httpd::Httpd(HobrasoftHttpd::HttpSettings *settings, QObject *parent) : HttpServer(settings, parent) {
}


HobrasoftHttpd::HttpRequestHandler *Httpd::requestHandler(HobrasoftHttpd::HttpConnection *connection) {
    RequestMapper *mapper = new RequestMapper(connection);
    connect(mapper, SIGNAL(sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,   SIGNAL(sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));
    return mapper;
}


Httpd *Httpd::create(QObject *parent, const QHostAddress& address, int port) {
    QSettings q;
    q.clear();
    q.setValue("http/address", address.toString());
    q.setValue("http/port",    port); 
    q.setValue("http/root",    "."); 
    q.setValue("http/sessionCookieName", "sessionid"); 
    q.setValue("http/threads", false);
    q.setValue("http/gzip",    false);
    q.sync();
    HttpSettings *hq = new HttpSettings(&q,parent);
    return new Httpd(hq,parent);
}


Httpd *Httpd::createSSL(QObject *parent, const QHostAddress& address, int port) {
    QSettings q;
    q.clear();
    q.setValue("http/address",  address.toString());
    q.setValue("http/port",     port); 
    q.setValue("http/root",     "."); 
    q.setValue("http/sessionCookieName", "sessionid"); 
    q.setValue("http/useSSL",   true);
    q.setValue("http/sslKey",   "://localhost.key");
    q.setValue("http/sslCrt",   "://localhost.crt");
    q.setValue("http/sslCaCrt", "://testCA.crt");
    q.setValue("http/IgnoreAllSslErrors", true);
    q.setValue("http/threads",  false);
    q.setValue("http/gzip",     false);
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

#define WAITFORREPLY() { \
    QEventLoop loop; \
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit())); \
    QTimer::singleShot(1000, &loop, SLOT(quit())); \
    loop.exec(); \
    QVERIFY(reply->isFinished()); \
    QVERIFY(reply->error() == QNetworkReply::NoError); \
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


void Test::httpServiceGet() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService   (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceGet(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url = QURL("hobrasofthttpd.test.html");
    #if QT_VERSION > 0x050000
    QUrlQuery qu;
    qu.addQueryItem("A", "1");
    qu.addQueryItem("B", "2");
    url.setQuery(qu);
    #else
    url.addQueryItem("A", "1");
    url.addQueryItem("B", "2");
    #endif
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    QNetworkReply *reply = manager.get(rq);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test");

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
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService    (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServicePost(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url = QURL("hobrasofthttpd.test.html");
    #if QT_VERSION > 0x050000
    QUrlQuery qu;
    qu.addQueryItem("A", "1");
    qu.addQueryItem("B", "2");
    url.setQuery(qu);
    #else
    url.addQueryItem("A", "1");
    url.addQueryItem("B", "2");
    #endif
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    rq.setRawHeader("Content-type", "text/text");
    QNetworkReply *reply = manager.post(rq, "abcd");
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test");

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
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService      (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceCookie(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url = QURL("hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    QNetworkCookie c("kuki", "xyz");
    rq.setHeader(QNetworkRequest::CookieHeader, QVariant::fromValue<QNetworkCookie>(c));
    QNetworkReply *reply = manager.get(rq);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test");

}


void Test::slotServiceUpload(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QByteArray load;
    for (int i=0; i<380000; i++) {
        load += i%256;
        }

    QVERIFY(request->uploadedFile("file1") != NULL);
    QVERIFY(request->contentType("file1") == "application/binary-test");
    QTemporaryFile *file1 = request->uploadedFile("file1");
    QVERIFY(file1 != NULL);
    QVERIFY(file1->readAll() == load);
    file1->close();
    file1->deleteLater();

    QVERIFY(request->uploadedFile("file2") != NULL);
    QVERIFY(request->contentType("file2") == "text/text");
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
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService      (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceUpload(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    // QByteArray abcd = "abcdefghijklmnopqrstuvwxyz0123457890\r\n";
    QByteArray load;
    for (int i=0; i<380000; i++) {
        load += i%256;
        // load += abcd[i%38];
        }

    QByteArray data;
    data += "--BOUNDARY\r\n";
    data += "Content-Disposition: form-data; name=\"file1\"; filename=\"file1.txt\"\r\n";
    data += "Content-Type: application/binary-test\r\n\r\n";
    data += load + "\r\n";
    data += "--BOUNDARY\r\n";
    data += "Content-Disposition: form-data; name=\"file2\"; filename=\"file2.txt\"\r\n";
    data += "Content-Type: text/text\r\n\r\n";
    data += "XYZ\r\n";
    data += "--BOUNDARY--\r\n";

    QUrl url = QURL("hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    rq.setRawHeader("Content-Type", "multipart/form-data; boundary=BOUNDARY");
    QNetworkReply *reply = manager.post(rq,data);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test");
}



void Test::slotServiceUploadInsensitive(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->header("Content-type") == "muLTIParT/fORM-DaTa; BOUndARy=BOUNDARY");

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


void Test::httpServiceUploadInsensitive() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService                 (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceUploadInsensitive(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

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

    QUrl url = QURL("hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    rq.setRawHeader("conTENt-TyPE", "muLTIParT/fORM-DaTa; BOUndARy=BOUNDARY");
    QNetworkReply *reply = manager.post(rq,data);
    WAITFORREPLY();
    QVERIFY(reply->readAll() == "test");
}


void Test::slotServiceResponse(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    Q_UNUSED(request);
    QVERIFY(response->headers().isEmpty() == true);
    QVERIFY(response->isConnected() == true);

    HttpCookie c("name", "value", 100, "/path", "Comment", "hobrasoft.cz", false);
    response->setCookie(c);
    QVERIFY(response->cookie("name").name()  ==  "name");
    QVERIFY(response->cookie("name").value() ==  "value");
    QVERIFY(response->cookies().size() == 1);

    response->clearHeaders();
    QVERIFY(response->headers().isEmpty() == true);
    response->setHeader("Size", 12);
    QVERIFY(response->headers()["Size"] == "12");
    QVERIFY(response->containsHeader("Size"));
    response->clearHeaders();
    QVERIFY(response->headers().isEmpty() == true);
    
    response->setHeader("Content-Type", "text/text");
    QVERIFY(response->headers().size() == 1);
    response->clearHeaders();
    QVERIFY(response->headers().isEmpty() == true);
    response->setHeader("Content-Type", "text/text");
    QVERIFY(response->headers().size() == 1);
    QVERIFY(response->headers()["Content-Type"] == "text/text");
    QVERIFY(response->containsHeader("Content-Type"));

    response->setStatus(404, "Not found");
    response->write("test");
    response->flush();

}


void Test::httpServiceResponse() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService        (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceResponse(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url = QURL("hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    QNetworkReply *reply = manager.get(rq);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::ContentNotFoundError);
    QVERIFY(reply->readAll() == "test"); 
    QVERIFY(reply->header(QNetworkRequest::ContentTypeHeader) == "text/text");
    QVERIFY(reply->header(QNetworkRequest::ContentLengthHeader) == 4);
    QVariant cv = reply->header(QNetworkRequest::SetCookieHeader);
    QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cv);
    QVERIFY(cookies.size() == 1);
    const QNetworkCookie& cookie = cookies[0];
    QVERIFY(cookie.name()  == "name");
    QVERIFY(cookie.value() == "value");
    QVERIFY(cookie.path()  == "/path");
    QVERIFY(cookie.isSecure() == false);
    QVERIFY(cookie.isHttpOnly() == false);
    QVERIFY(cookie.expirationDate() > QDateTime::currentDateTime().addSecs(90));
    QVERIFY(cookie.expirationDate() < QDateTime::currentDateTime().addSecs(110));
    QVERIFY(cookie.domain() == "hobrasoft.cz");

}


void Test::slotServiceResponseChunked(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    Q_UNUSED(request);
    QVERIFY(response->headers().isEmpty() == true);
    QVERIFY(response->isConnected() == true);

    response->setHeader("Transfer-Encoding", "chunked");

    for (int i=0; i<10; i++) {
        QString chunk = QString("%1").arg(i);
        response->write(chunk.toUtf8());
        response->flush();
        }

    response->flushAndClose();
}


void Test::httpServiceResponseChunked() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService               (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceResponseChunked(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QUrl url = QURL("hobrasofthttpd.test.html");
    QNetworkAccessManager manager(this);
    QNetworkRequest rq(url);
    QNetworkReply *reply = manager.get(rq);

    WAITFORREPLY();
    QVERIFY(reply->readAll() == "0123456789"); 

}


void Test::slotServiceSession(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(m_sessionStore != NULL);
    HttpSession session = m_sessionStore->session(request, response);
    if (request->path() == "/1") {
        session.add("A", "1");
        }
    if (request->path() == "/2") {
        QVERIFY(session.contains("A") == true);
        QVERIFY(session.value("A") == "1");
        session.remove("A");
        QVERIFY(session.contains("A") == false);
        session.add("B", "2");
        QVERIFY(session.contains("B") == true);
        }
    if (request->path() == "/3") {
        QVERIFY(session.contains("A") == false);
        QVERIFY(session.contains("B") == true);
        QVERIFY(session.value("B") == "2");
        }
    response->flush();
}


void Test::httpServiceSession() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    m_sessionStore = new HttpSessionStore(server);
    QVERIFY(m_sessionStore != NULL);
    QVERIFY(m_sessionStore->sessionCookieName() == "sessionid");
    connect(server, SIGNAL( sigService       (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceSession(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QNetworkReply *reply;
    QNetworkAccessManager manager(this);
    reply = manager.get(QNetworkRequest(QURL("1")));
    WAITFORREPLY();

    QVariant cv = reply->header(QNetworkRequest::SetCookieHeader);
    QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(cv);
    QVERIFY(cookies.size() == 1);
    const QNetworkCookie& cookie = cookies[0];
    QVERIFY(cookie.name()  == "sessionid");

    reply = manager.get(QNetworkRequest(QURL("2")));
    WAITFORREPLY();

    reply = manager.get(QNetworkRequest(QURL("3")));
    WAITFORREPLY();

}


void Test::slotServiceAbort(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    Q_UNUSED(request);
    Q_UNUSED(response);
    QTest::qWait(200);
    response->write("abcd");
    response->flush();
}


void TestWorker::start() {
    QNetworkAccessManager manager(this);
    QNetworkReply *reply = manager.get(QNetworkRequest(QURL("ServiceAbort")));
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));
    QTimer::singleShot(100, &loop, SLOT(quit()));
    loop.exec();
    reply->abort();
}


// This test does not work properly
// The thread is not started
// Application should not SIGSEGV here
void Test::httpServiceAbort() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService      (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotServiceAbort (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));
    TestWorker *worker = new TestWorker(m_localhost, m_port);
    worker->moveToThread(&m_workerThread);
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater())); // Signal is not signalled to different thread??!!
    QTimer::singleShot(1, worker, SLOT(start()));
    QCoreApplication::processEvents();

    m_workerThread.wait(3000);

    QTest::qWait(300);
}


void Test::slotStaticFileController(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    StaticFileController *sfc = new StaticFileController(request->connection());
    sfc->addMimeType("tiff", "image/tiff");
    QVERIFY(sfc->mimetype("png")   == "image/png");
    QVERIFY(sfc->mimetype("jpeg")  == "image/jpeg");
    QVERIFY(sfc->mimetype("jpg")   == "image/jpeg");
    QVERIFY(sfc->mimetype("gif")   == "image/gif");
    QVERIFY(sfc->mimetype("tiff")  == "image/tiff");
    QVERIFY(sfc->mimetype("txt")   == "text/plain");
    QVERIFY(sfc->mimetype("html")  == "text/html");
    QVERIFY(sfc->mimetype("xhtml") == "text/html");
    QVERIFY(sfc->mimetype("htm")   == "text/html");
    QVERIFY(sfc->mimetype("css")   == "text/css");
    QVERIFY(sfc->mimetype("json")  == "application/json");
    QVERIFY(sfc->mimetype("js")    == "application/javascript");

    QVERIFY(sfc->settings() == request->connection()->settings());

    sfc->service(request, response);

}


void Test::staticFileController() {
    QObject qobject(0);
    Httpd *server = Httpd::create(&qobject, m_localhost, m_port);
    HttpSettings *settings = const_cast<HttpSettings *>(server->settings());
    settings->setMaxAge(3000);
    QVERIFY(server != NULL);
    connect(server, SIGNAL( sigService             (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotStaticFileController(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QFile file("./hobrasofthttpd.test.html");
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    QNetworkReply *reply;
    QNetworkAccessManager manager(this);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));

    #if QT_VERSION > 0x050000
    QVERIFY(file.setPermissions(QFileDevice::WriteUser) == true);
    #else
    QVERIFY(file.setPermissions(QFile::WriteUser) == true);
    #endif
    reply = manager.get(QNetworkRequest(QURL("hobrasofthttpd.test.html")));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    #if QT_VERSION > 0x050000
    QVERIFY(reply->error() == QNetworkReply::ContentAccessDenied);
    #else
    QVERIFY(reply->error() == QNetworkReply::ContentOperationNotPermittedError);
    #endif

    #if QT_VERSION > 0x050000
    QVERIFY(file.setPermissions(QFileDevice::WriteUser|QFileDevice::ReadUser) == true);
    #else
    QVERIFY(file.setPermissions(QFile::WriteUser|QFile::ReadUser) == true);
    #endif
    reply = manager.get(QNetworkRequest(QURL("hobrasofthttpd.test.html")));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll() == "test");
    QVERIFY(reply->header(QNetworkRequest::ContentTypeHeader) == "text/html");
    QVERIFY(reply->hasRawHeader("Cache-Control") == true);
    QVERIFY(reply->rawHeader("Cache-Control") == "Public,max-age=3000");

    reply = manager.get(QNetworkRequest(QURL("../hobrasofthttpd.test.html")));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    #if QT_VERSION > 0x050000
    QVERIFY(reply->error() == QNetworkReply::ContentAccessDenied);
    #else
    QVERIFY(reply->error() == QNetworkReply::ContentOperationNotPermittedError);
    #endif

    QUrl url = QURL("hobrasofthttpd.test.html");
    #if QT_VERSION > 0x050000
    QUrlQuery qu;
    qu.addQueryItem("CheckCache", "true");
    url.setQuery(qu);
    #else
    url.addQueryItem("CheckCache", "true");
    #endif
    reply = manager.get(QNetworkRequest(url));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);

}


void Test::slotHttpSSL(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->connection()->commonName() == "client");
    QVERIFY(request->connection()->verified() == true);
    QSslCertificate crt = request->connection()->peerCertificate();

    response->write("test");
    response->flush();
}


void Test::httpSSL() {
    QObject qobject(0);
    Httpd *server = Httpd::createSSL(&qobject, m_localhost, m_port);
    connect(server, SIGNAL( sigService(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotHttpSSL(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QNetworkRequest request;
    QNetworkReply *reply;
    QNetworkAccessManager manager(this);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));

    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> caCertificates;
    QFile fileca(":/testCA.crt");
    QVERIFY(fileca.open(QIODevice::ReadOnly) == true);
    QSslCertificate cacrt(&fileca);
    QVERIFY(cacrt.isNull() == false);
    #if QT_VERSION > 0x050000
    QVERIFY(cacrt.subjectInfo(QSslCertificate::Organization).isEmpty()           == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::LocalityName).isEmpty()           == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::OrganizationalUnitName).isEmpty() == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::CountryName).isEmpty()            == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::StateOrProvinceName).isEmpty()    == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::Organization)[0]                  == "Hobrasoft s.r.o.");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::LocalityName)[0]                  == "Roznov pod Radhostem");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::OrganizationalUnitName)[0]        == "IT");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::CountryName)[0]                   == "CZ");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::StateOrProvinceName)[0]           == "Czech republic");
    #else
    QVERIFY(cacrt.subjectInfo(QSslCertificate::Organization)                     == "Hobrasoft s.r.o.");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::LocalityName)                     == "Roznov pod Radhostem");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::OrganizationalUnitName)           == "IT");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::CountryName)                      == "CZ");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::StateOrProvinceName)              == "Czech republic");
    #endif
    caCertificates << cacrt;
    configuration.setCaCertificates(caCertificates);
    fileca.close();

    QFile filekey(":/client.key");
    QVERIFY(filekey.open(QIODevice::ReadOnly) == true);
    QSslKey key(&filekey, QSsl::Rsa, QSsl::Pem);
    filekey.close();
    QVERIFY(key.isNull() == false);
    QVERIFY(key.algorithm() == QSsl::Rsa);
    QVERIFY(key.type() == QSsl::PrivateKey);
    configuration.setPrivateKey(key);

    QFile filecrt(":/client.crt");
    QVERIFY(filecrt.open(QIODevice::ReadOnly) == true);
    QSslCertificate crt(&filecrt);
    QVERIFY(crt.isNull() == false);
    #if QT_VERSION > 0x050000
    QVERIFY(crt.subjectInfo(QSslCertificate::Organization).isEmpty()           == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::LocalityName).isEmpty()           == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::CommonName).isEmpty()             == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::OrganizationalUnitName).isEmpty() == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::CountryName).isEmpty()            == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::StateOrProvinceName).isEmpty()    == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::Organization)[0]                  == "Hobrasoft s.r.o.");
    QVERIFY(crt.subjectInfo(QSslCertificate::CommonName)[0]                    == "client");
    QVERIFY(crt.subjectInfo(QSslCertificate::LocalityName)[0]                  == "Roznov pod Radhostem");
    QVERIFY(crt.subjectInfo(QSslCertificate::OrganizationalUnitName)[0]        == "IT");
    QVERIFY(crt.subjectInfo(QSslCertificate::CountryName)[0]                   == "CZ");
    QVERIFY(crt.subjectInfo(QSslCertificate::StateOrProvinceName)[0]           == "Czech republic");
    #else
    QVERIFY(crt.subjectInfo(QSslCertificate::CommonName)                       == "client");
    QVERIFY(crt.subjectInfo(QSslCertificate::Organization)                     == "Hobrasoft s.r.o.");
    QVERIFY(crt.subjectInfo(QSslCertificate::LocalityName)                     == "Roznov pod Radhostem");
    QVERIFY(crt.subjectInfo(QSslCertificate::OrganizationalUnitName)           == "IT");
    QVERIFY(crt.subjectInfo(QSslCertificate::CountryName)                      == "CZ");
    QVERIFY(crt.subjectInfo(QSslCertificate::StateOrProvinceName)              == "Czech republic");
    #endif
    configuration.setLocalCertificate(crt);
    filecrt.close();

    request = QNetworkRequest(QURLSSL("1"));
    request.setSslConfiguration(configuration);
    reply = manager.get(request);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);

}


void Test::slotSslErrors(QNetworkReply *reply, QList<QSslError> errors) {
    Q_UNUSED(errors);
    reply->ignoreSslErrors();
}


void Test::slotHttpSSLselfsigned(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    QVERIFY(request->connection()->commonName() == "selfsigned");
    QVERIFY(request->connection()->verified() == false);
    QSslCertificate crt = request->connection()->peerCertificate();

    response->write("test");
    response->flush();
}


void Test::httpSSLselfsigned() {
    QObject qobject(0);
    Httpd *server = Httpd::createSSL(&qobject, m_localhost, m_port);
    connect(server, SIGNAL( sigService          (HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)),
            this,     SLOT(slotHttpSSLselfsigned(HobrasoftHttpd::HttpRequest *, HobrasoftHttpd::HttpResponse *)));

    QNetworkRequest request;
    QNetworkReply *reply;
    QNetworkAccessManager manager(this);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop, SLOT(quit()));

    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> caCertificates;
    QFile fileca(":/selfsigned.crt");
    QVERIFY(fileca.open(QIODevice::ReadOnly) == true);
    QSslCertificate cacrt(&fileca);
    QVERIFY(cacrt.isNull() == false);
    #if QT_VERSION > 0x050000
    QVERIFY(cacrt.subjectInfo(QSslCertificate::Organization).isEmpty()           == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::LocalityName).isEmpty()           == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::OrganizationalUnitName).isEmpty() == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::CountryName).isEmpty()            == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::StateOrProvinceName).isEmpty()    == false);
    QVERIFY(cacrt.subjectInfo(QSslCertificate::Organization)[0]                  == "Hobrasoft s.r.o.");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::LocalityName)[0]                  == "Roznov pod Radhostem");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::OrganizationalUnitName)[0]        == "IT");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::CountryName)[0]                   == "CZ");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::StateOrProvinceName)[0]           == "Czech republic");
    #else
    QVERIFY(cacrt.subjectInfo(QSslCertificate::Organization)                     == "Hobrasoft s.r.o.");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::LocalityName)                     == "Roznov pod Radhostem");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::OrganizationalUnitName)           == "IT");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::CountryName)                      == "CZ");
    QVERIFY(cacrt.subjectInfo(QSslCertificate::StateOrProvinceName)              == "Czech republic");
    #endif
    caCertificates << cacrt;
    configuration.setCaCertificates(caCertificates);
    fileca.close();

    QFile filekey(":/selfsigned.key");
    QVERIFY(filekey.open(QIODevice::ReadOnly) == true);
    QSslKey key(&filekey, QSsl::Rsa, QSsl::Pem);
    filekey.close();
    QVERIFY(key.isNull() == false);
    QVERIFY(key.algorithm() == QSsl::Rsa);
    QVERIFY(key.type() == QSsl::PrivateKey);
    configuration.setPrivateKey(key);

    QFile filecrt(":/selfsigned.crt");
    QVERIFY(filecrt.open(QIODevice::ReadOnly) == true);
    QSslCertificate crt(&filecrt);
    QVERIFY(crt.isNull() == false);
    #if QT_VERSION > 0x050000
    QVERIFY(crt.subjectInfo(QSslCertificate::Organization).isEmpty()           == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::LocalityName).isEmpty()           == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::CommonName).isEmpty()             == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::OrganizationalUnitName).isEmpty() == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::CountryName).isEmpty()            == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::StateOrProvinceName).isEmpty()    == false);
    QVERIFY(crt.subjectInfo(QSslCertificate::Organization)[0]                  == "Hobrasoft s.r.o.");
    QVERIFY(crt.subjectInfo(QSslCertificate::CommonName)[0]                    == "selfsigned");
    QVERIFY(crt.subjectInfo(QSslCertificate::LocalityName)[0]                  == "Roznov pod Radhostem");
    QVERIFY(crt.subjectInfo(QSslCertificate::OrganizationalUnitName)[0]        == "IT");
    QVERIFY(crt.subjectInfo(QSslCertificate::CountryName)[0]                   == "CZ");
    QVERIFY(crt.subjectInfo(QSslCertificate::StateOrProvinceName)[0]           == "Czech republic");
    #else
    QVERIFY(crt.subjectInfo(QSslCertificate::CommonName)                       == "selfsigned");
    QVERIFY(crt.subjectInfo(QSslCertificate::Organization)                     == "Hobrasoft s.r.o.");
    QVERIFY(crt.subjectInfo(QSslCertificate::LocalityName)                     == "Roznov pod Radhostem");
    QVERIFY(crt.subjectInfo(QSslCertificate::OrganizationalUnitName)           == "IT");
    QVERIFY(crt.subjectInfo(QSslCertificate::CountryName)                      == "CZ");
    QVERIFY(crt.subjectInfo(QSslCertificate::StateOrProvinceName)              == "Czech republic");
    #endif
    configuration.setLocalCertificate(crt);
    filecrt.close();

    request = QNetworkRequest(QURLSSL("1"));
    request.setSslConfiguration(configuration);
    reply = manager.get(request);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::SslHandshakeFailedError);
    reply->deleteLater();

    QEventLoop loop2;
    connect(&manager, SIGNAL(finished(QNetworkReply *)), &loop2, SLOT(quit()));
    connect(&manager, SIGNAL(    sslErrors(QNetworkReply *, QList<QSslError>)),
            this,       SLOT(slotSslErrors(QNetworkReply *, QList<QSslError>)));
    reply = manager.get(request);
    QTimer::singleShot(10000, &loop2, SLOT(quit()));
    loop2.exec();
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->isRunning() == false);
    QVERIFY(reply->isFinished());

}


