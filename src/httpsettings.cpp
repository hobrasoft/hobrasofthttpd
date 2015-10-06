/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpsettings.h"
#include <QSettings>

/**
 * @addtogroup konfigurace
 * @{
 *
 * Configuration HobrasoftHttpd::HttpServer
 * ---------
 *
 * - __httpd/address__ - bind address for http server (::)
 * - __httpd/port__ - bind port for http server (8080)
 * - __httpd/timeout__ - timeout for http request (600 sec)
 * - __httpd/maxRequestSize__ - maximum size of request (16384)
 * - __httpd/maxMultiPartSize__ - maximum size of multipart request (1048576)
 * - __httpd/tempDir__ - temporary directory (/tmp)
 * - __httpd/sessionExpirationTime__ - session timeout (3600 sec)
 * - __httpd/sessionIdName__ - session cookie name (sessionid)
 * - __httpd/encoding__ - encoding set in headers (UTF-8)
 * - __httpd/root__ - document root directory (.)
 * - __httpd/cacheMaxAge__ - maximum age of objects in cache (3600 sec)
 * - __httpd/maxCacheFileSize__ - maximum size of files in cache (1000000)
 * - __httpd/indexFile__ - index file in direcotry (index.html)
 * - __httpd/cacheOn__ - on/off cache (false)
 * - __httpd/peristent__ - set or unset persistent connections (false)
 *
 * @}
 */

using namespace HobrasoftHttpd;


HttpSettings::HttpSettings(QObject *parent) : QObject(parent) {
    m_port                  = 8080;
    m_address               = QHostAddress::AnyIPv6;
    m_timeout               = 600;
    m_maxAge                = 3600;
    m_encoding              = "UTF-8";
    m_docroot               = ".";
    m_maxCachedFileSize     = 1000000;
    m_cacheTimeout          = 600000;
    m_indexFile             = "index.html";
    m_cacheOn               = false;
    m_sessionCookieName     = "sessionid";
    m_sessionExpirationTime = 3600;
    m_maxRequestSize        = 16384;
    m_maxMultiPartSize      = 16728064;
    m_persistent            = true;
}


void HttpSettings::readSettings(const QSettings *settings, const QString& section) {
    QString section2 = "http";
    m_port                  = settings->value(section  + "/port",
                              settings->value(section2 + "/port",                  80)).toInt();
    m_address               = settings->value(section  + "/address",
                              settings->value(section2 + "/address",               "::")).toString();
    m_timeout               = settings->value(section  + "/timeout",
                              settings->value(section2 + "/timeout",               600)).toInt();
    m_maxAge                = settings->value(section  + "/maxAge", 
                              settings->value(section2 + "/maxAge",                3600)).toInt();
    m_encoding              = settings->value(section  + "/encoding", 
                              settings->value(section2 + "/encoding",              "UTF-8")).toString();
    m_docroot               = settings->value(section  + "/root", 
                              settings->value(section2 + "/root",                  ".")).toString();
    m_maxCachedFileSize     = settings->value(section  + "/maxCachedFileSize", 
                              settings->value(section2 + "/maxCachedFileSize",     1000000)).toInt();
    m_cacheTimeout          = settings->value(section  + "/cacheTimeout", 
                              settings->value(section2 + "/cacheTimeout",          600000)).toInt();
    m_indexFile             = settings->value(section  + "/indexFile", 
                              settings->value(section2 + "/indexFile",             "index.html")).toString();
    m_cacheOn               = settings->value(section  + "/cacheOn", 
                              settings->value(section2 + "/cacheOn",               false)).toBool();
    m_sessionCookieName     = settings->value(section  + "/sessionCookieName", 
                              settings->value(section2 + "/sessionCookieName",     "sessionid")).toString();
    m_sessionExpirationTime = settings->value(section  + "/sessionExpirationTime", 
                              settings->value(section2 + "/sessionExpirationTime", 3600)).toInt();
    m_maxRequestSize        = settings->value(section  + "/maxRequestSize", 
                              settings->value(section2 + "/maxRequestSize",        16384)).toInt();
    m_maxMultiPartSize      = settings->value(section  + "/maxMultiPartSize", 
                              settings->value(section2 + "/maxMultiPartSize",      16728064)).toInt();
    m_persistent            = settings->value(section  + "/persistent",
                              settings->value(section2 + "/persistent",            true)).toBool();
}


HttpSettings::HttpSettings(const QSettings *settings, const QString& section, QObject *parent) : QObject(parent) {
    readSettings(settings, section);
}


HttpSettings::HttpSettings(const QSettings *settings, QObject *parent) : QObject(parent) {
    readSettings(settings, "http");
}



