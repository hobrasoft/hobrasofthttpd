/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "test.h"
#include "httpsettings.h"
#include "httpserver.h"
#include "httptcpserver.h"

#include <QDebug>
#include <QTcpSocket>
#include <QSignalSpy>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
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


void Test::createServer(QObject *parent, const QHostAddress& address) {
    QSettings q;
    q.clear();
    q.setValue("http/address", address.toString());
    q.setValue("http/port",    8080); 
    q.setValue("http/root",    "."); 
    q.sync();
    HttpSettings *hq = new HttpSettings(&q,parent);
    QVERIFY(hq->port()    == 8080);
    QVERIFY(hq->address() == address);
    HttpServer *s = new HttpServer(hq,parent);
    QVERIFY(s->m_server != NULL);
    QVERIFY(s->m_settings != NULL);
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
    connect(&manager, SIGNAL(finished(QNetworkReply *)),
            &loop,      SLOT(quit()));
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
    connect(&manager, SIGNAL(finished(QNetworkReply *)),
            &loop,      SLOT(quit()));
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
    connect(&manager, SIGNAL(finished(QNetworkReply *)),
            &loop,      SLOT(quit()));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QVERIFY(reply->isFinished());
    QVERIFY(reply->error() == QNetworkReply::NoError);
    QVERIFY(reply->readAll()  == "test");
    file.remove();
    filei.remove();
}


