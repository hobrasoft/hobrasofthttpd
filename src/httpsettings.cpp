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
 * Konfigurace HobrasoftHttpd::HttpServer
 * ---------
 *
 * - __httpd/address__ - bind adresa pro http server (0.0.0.0)
 * - __httpd/port__ - bind port pro http server (8086)
 * - __httpd/timeout__ - timeout pro http požadavek, než se uzavře spojení při neaktivitě (600 sec)
 * - __httpd/maxRequestSize__ - maximání velikost požadavku (16384)
 * - __httpd/maxMultiPartSize__ - maximální velikost požadavku při multipart (1048576)
 * - __httpd/tempDir__ - dočasný adresář (/tmp)
 * - __httpd/sessionExpirationTime__ - čas pro vypršení session (3600 sec)
 * - __httpd/sessionIdName__ - jméno cookie pro předávání session (sessionid)
 * - __httpd/encoding__ - kódování předávané v hlavičce (UTF-8)
 * - __httpd/root__ - documentroot adresář (.)
 * - __httpd/cacheMaxAge__ -  (3600 sec)
 * - __httpd/maxCacheFileSize__ - maximální velikost soubor v cache (1000000)
 * - __httpd/indexFile__ - index v adresáři (index.html)
 * - __httpd/cacheOn__ - zapnutí/vypnutí cache (false)
 *
 * @}
 */

using namespace HobrasoftHttpd;


HttpSettings::HttpSettings(QObject *parent) : QObject(parent) {
    m_port                  = 8080;
    m_address               = QHostAddress("::");
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
    m_maxMultiPartSize      = 1048576;
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
                              settings->value(section2 + "/maxMultiPartSize",      1048576)).toInt();
}


HttpSettings::HttpSettings(const QSettings *settings, const QString& section, QObject *parent) : QObject(parent) {
    readSettings(settings, section);
}


HttpSettings::HttpSettings(const QSettings *settings, QObject *parent) : QObject(parent) {
    readSettings(settings, "http");
}



