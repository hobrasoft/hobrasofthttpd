/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpsettings.h"
#include <QSettings>

/**
 * @addtogroup configuration
 * @{
 *
 * Configuration HobrasoftHttpd::HttpServer
 * ---------
 *
 * - __httpd/address__ - bind address for http server (::)
 * - __httpd/port__ - bind port for http server (8080)
 * - __httpd/timeout__ - timeout for http request (600 sec)
 * - __httpd/maxAge__ - maximum age for browser cache or caching proxy server for static files (3600 sec)
 * - __httpd/maxRequestSize__ - maximum size of request (16384)
 * - __httpd/maxMultiPartSize__ - maximum size of multipart request (1048576)
 * - __httpd/tempDir__ - temporary directory (/tmp)
 * - __httpd/sessionExpirationTime__ - session timeout (3600 sec)
 * - __httpd/sessionIdName__ - session cookie name (sessionid)
 * - __httpd/encoding__ - encoding set in headers (UTF-8)
 * - __httpd/root__ - document root directory (.)
 * - __httpd/cacheMaxAge__ - maximum age of objects in cache (3600 sec)
 * - __httpd/indexFile__ - index file in direcotry (index.html)
 * - __httpd/useSSL__  - on/off SSL connection (off)
 * - __httpd/sslKey__  - path to SSL key file in PEM format
 * - __httpd/sslCrt__  - path to SSL certificate file in PEM format
 * - __httpd/sslCaCrt__  - path to SSL CA certificate file in PEM format
 *
 * SSL errors
 * ----------
 * All parameters has default value false except IgnoreAllSslErrors
 * - __httpd/IgnoreAllSslErrors__  - default true
 * - __httpd/IgnoreUnableToGetIssuerCertificate__
 * - __httpd/IgnoreUnableToDecryptCertificateSignature__
 * - __httpd/IgnoreUnableToDecodeIssuerPublicKey__
 * - __httpd/IgnoreCertificateSignatureFailed__
 * - __httpd/IgnoreCertificateNotYetValid__
 * - __httpd/IgnoreCertificateExpired__
 * - __httpd/IgnoreInvalidNotBeforeField__
 * - __httpd/IgnoreInvalidNotAfterField__
 * - __httpd/IgnoreSelfSignedCertificate__
 * - __httpd/IgnoreSelfSignedCertificateInChain__
 * - __httpd/IgnoreUnableToGetLocalIssuerCertificate__
 * - __httpd/IgnoreUnableToVerifyFirstCertificate__
 * - __httpd/IgnoreCertificateRevoked__
 * - __httpd/IgnoreInvalidCaCertificate__
 * - __httpd/IgnorePathLengthExceeded__
 * - __httpd/IgnoreInvalidPurpose__
 * - __httpd/IgnoreCertificateUntrusted__
 * - __httpd/IgnoreCertificateRejected__
 * - __httpd/IgnoreSubjectIssuerMismatch__
 * - __httpd/IgnoreAuthorityIssuerSerialNumberMismatch__
 * - __httpd/IgnoreNoPeerCertificate__
 * - __httpd/IgnoreHostNameMismatch__
 * - __httpd/IgnoreUnspecifiedError__
 * - __httpd/IgnoreNoSslSupport__
 * - __httpd/IgnoreCertificateBlacklisted__
 *
 * @}
 */

int qHash(const QSslError error) {
    return qHash(error.error());
}

using namespace HobrasoftHttpd;


HttpSettings::HttpSettings(QObject *parent) : QObject(parent) {
    m_port                  = 8080;
    m_address               = QHostAddress::AnyIPv6;
    m_timeout               = 600;
    m_maxAge                = 3600;
    m_encoding              = "UTF-8";
    m_docroot               = ".";
    m_indexFile             = "index.html";
    m_sessionCookieName     = "sessionid";
    m_sessionExpirationTime = 3600;
    m_maxRequestSize        = 16384;
    m_maxMultiPartSize      = 16728064;
    m_useSSL                = false;
}


void HttpSettings::readSettings(const QSettings *settings, const QString& section, const QString& section2) {
    m_section2 = section2;
    readSettings(settings, section);
}


void HttpSettings::readSettings(const QSettings *settings, const QString& section) {
    m_port                  = settings->value(  section  + "/port",
                              settings->value(m_section2 + "/port",                  80)).toInt();
    m_address               = settings->value(  section  + "/address",
                              settings->value(m_section2 + "/address",               "::")).toString();
    m_timeout               = settings->value(  section  + "/timeout",
                              settings->value(m_section2 + "/timeout",               600)).toInt();
    m_maxAge                = settings->value(  section  + "/maxAge", 
                              settings->value(m_section2 + "/maxAge",                3600)).toInt();
    m_encoding              = settings->value(  section  + "/encoding", 
                              settings->value(m_section2 + "/encoding",              "UTF-8")).toString();
    m_docroot               = settings->value(  section  + "/root", 
                              settings->value(m_section2 + "/root",                  ".")).toString();
    m_indexFile             = settings->value(  section  + "/indexFile", 
                              settings->value(m_section2 + "/indexFile",             "index.html")).toString();
    m_sessionCookieName     = settings->value(  section  + "/sessionCookieName", 
                              settings->value(m_section2 + "/sessionCookieName",     "sessionid")).toString();
    m_sessionExpirationTime = settings->value(  section  + "/sessionExpirationTime", 
                              settings->value(m_section2 + "/sessionExpirationTime", 3600)).toInt();
    m_maxRequestSize        = settings->value(  section  + "/maxRequestSize", 
                              settings->value(m_section2 + "/maxRequestSize",        16384)).toInt();
    m_maxMultiPartSize      = settings->value(  section  + "/maxMultiPartSize", 
                              settings->value(m_section2 + "/maxMultiPartSize",      16728064)).toInt();
    m_useSSL                = settings->value(  section  + "/useSSL",
                              settings->value(m_section2 + "/useSSL",                false)).toBool();
    m_sslKey                = settings->value(  section  + "/sslKey",
                              settings->value(m_section2 + "/sslKey",                "")).toString();
    m_sslCrt                = settings->value(  section  + "/sslCrt",
                              settings->value(m_section2 + "/sslCrt",                "")).toString();
    m_sslCaCrt              = settings->value(  section  + "/sslCaCrt",
                              settings->value(m_section2 + "/sslCaCrt",              "")).toString();
    m_ignoreAllSslErrors    = settings->value(  section  + "/IgnoreAllSslErrors",
                              settings->value(m_section2 + "/IgnoreAllSslErrors",    true)).toBool();

    #define SSLERROR(x) { if (settings->value(  section  + "/Ignore" + #x, \
                              settings->value(m_section2 + "/Ignore" + #x, false)).toBool()) { \
                                m_sslErrors << QSslError::x; \
                                } \
                        }

    SSLERROR(UnableToGetIssuerCertificate);
    SSLERROR(UnableToDecryptCertificateSignature);
    SSLERROR(UnableToDecodeIssuerPublicKey);
    SSLERROR(CertificateSignatureFailed);
    SSLERROR(CertificateNotYetValid);
    SSLERROR(CertificateExpired);
    SSLERROR(InvalidNotBeforeField);
    SSLERROR(InvalidNotAfterField);
    SSLERROR(SelfSignedCertificate);
    SSLERROR(SelfSignedCertificateInChain);
    SSLERROR(UnableToGetLocalIssuerCertificate);
    SSLERROR(UnableToVerifyFirstCertificate);
    SSLERROR(CertificateRevoked);
    SSLERROR(InvalidCaCertificate);
    SSLERROR(PathLengthExceeded);
    SSLERROR(InvalidPurpose);
    SSLERROR(CertificateUntrusted);
    SSLERROR(CertificateRejected);
    SSLERROR(SubjectIssuerMismatch);
    SSLERROR(AuthorityIssuerSerialNumberMismatch);
    SSLERROR(NoPeerCertificate);
    SSLERROR(HostNameMismatch);
    SSLERROR(UnspecifiedError);
    SSLERROR(NoSslSupport);
    #if QT_VERSION > 0x040700
    SSLERROR(CertificateBlacklisted);
    #endif

}


HttpSettings::HttpSettings(const QSettings *settings, const QString& section, QObject *parent) : QObject(parent) {
    QString m_section2 = "http";
    readSettings(settings, section);
}


HttpSettings::HttpSettings(const QSettings *settings, QObject *parent) : QObject(parent) {
    QString m_section2 = "http";
    readSettings(settings, "http");
}


bool HttpSettings::ignoreSslError(QSslError error) const {
    if (m_ignoreAllSslErrors) { 
        return true;
        }
    return m_sslErrors.contains(error);
}


