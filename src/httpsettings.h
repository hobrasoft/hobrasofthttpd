/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HTTP_SETTINGS_H_
#define _HTTP_SETTINGS_H_

#include <QHostAddress>
#include <QSettings>

namespace HobrasoftHttpd {

/*
@brief Configuration of the http server instance

Default configuration is read from current QSettings. Before you start your HttpServer you have to set
the organization name, domain name and application name. In the configuration the group [httpd] is 
found and configuration parameters from this section is used. Configuration file (when using ini format)
can looks like this:
 
@code
[httpd]
address = 0.0.0.0
port = 8086
root = /home/tycho/SRC/mdcv04ip/htdocs
cacheMaxAge = 600000
indexFile = index.html
encoding = UTF-8
maxCachedFileSize = 1000000
cacheTime = 600000
cacheOn = false
timeout = 60000
@endcode

Configuration parameters
------------------------

- __address__ - listening bind address, 0.0.0.0, 192.168.1.0, 127.0.0.1 or ::1
- __port__ - port number
- __root__ - path to document root. Can be absolute or relative to current working directory
- __chacheMaxAge__ - maximum age for static files in cache
- __indexFile__ - name of index file in directory (index.html)
- __encoding__ - encoding in Content-type header (default utf-8)
- __maxCacheFileSize__ - maximum file size for static files in cache
- __cacheOn__ - if true, the cache is on
- __cacheTimeout__ - the object in cache is valid only if it is not older then the cacheTimeout
- __timeout__ - timeout for opened connection from clients
- __maxRequestSize__ - maximum size of request
- __maxMultiPartSize__ - maximum size of multipart requests
- __tempDir__ - path to temporary directory (/tmp)
- __persistent__ - of/off persistent connection for event streams

*/
class HttpSettings : public QObject {
    Q_OBJECT
  public:

    /**
     * @brief Constructor reads default QSettings file section [httpd]
     */
    HttpSettings(QObject *parent);

    /**
     * @brief Constructor reads settings from parameters, uses default section [httpd]
     */
    HttpSettings(const QSettings *, QObject *parent);

    /**
     * @brief Constructor reads settings from parameters, uses custom section name
     *
     * When the parameter could not be read in the section, alternate section name is used [http]
     */
    HttpSettings(const QSettings *, const QString& section, QObject *parent);

    const QHostAddress&  address() const { return m_address; }                              ///< Returns the bind address
    void            setAddress(const QHostAddress& x) { m_address = x; }                    ///< Sets the bind address

    int             port() const { return m_port; }                                         ///< Returns the port number 
    void            setPort(int x) { m_port = x; }                                          ///< Sets the port number

    int             timeout() const { return m_timeout; }                                   ///< Returns timeout for opened connection from clients 
    void            setTimeout(int x) { m_timeout = x; }                                    ///< Sets timeout for opened connection from clients

    int             maxAge() const { return m_maxAge; }                                     ///< Returns the max age for cache objects 
    void            setMaxAge(int x) { m_maxAge = x; }                                      ///< Sets the max age cache objects

    const QString&  encoding() const { return m_encoding; }                                 ///< Returns the encoding in COntent-type header
    void            setEncoding(const QString& x) { m_encoding = x; }                       ///< Sets the encoding in COntent-type header

    const QString&  docroot() const { return m_docroot; }                                   ///< Returns path to document root 
    void            setDocroot(const QString& x) { m_docroot = x; }                         ///< Sets path to document root

    int             maxCachedFileSize() const { return m_maxCachedFileSize; }               ///< Returns maximum cached file size
    void            setMaxCachedFileSize(int x) { m_maxCachedFileSize = x; }                ///< Sets maximum cached file size

    const QString&  indexFile() const { return m_indexFile; }                               ///< Returns name of index file in directory (index.html) 
    void            setIndexFile(const QString& x) { m_indexFile = x; }                     ///< Sets name of index file in directory (index.html)

    int             cacheTimeout() const { return m_cacheTimeout; }                         ///< Returns the maximum life time for valid documents in cache
    void            setCacheTimeout(int x) { m_cacheTimeout = x; }                          ///< Sets the maximum life time for valid documents if cache

    bool            cacheOn() const { return m_cacheOn; }                                   ///< Returns true if the cache is on
    void            setCacheOn(bool x) { m_cacheOn = x; }                                   ///< Sets the cache on or off

    int             sessionExpirationTime() const { return m_sessionExpirationTime; }       ///< Returns maximum expiration time for the session
    void            setExpirationTime(int x) { m_sessionExpirationTime = x; }               ///< Sets maximum expiration time for the session

    const QString&  sessionCookieName() const { return m_sessionCookieName; }               ///< Returns session cookie name 
    void            setSessionCookieName(const QString& x) { m_sessionCookieName = x; }     ///< Sets session cookie name

    int             maxRequestSize() const { return m_maxRequestSize; }                     ///< Returns maximum request size 
    void            setMaxRequestSize(int x) { m_maxRequestSize = x; }                      ///< Sets maximum request size

    int             maxMultiPartSize() const { return m_maxMultiPartSize; }                 ///< Returns maximum request size in multipart requests 
    void            setMaxMultiPartSize(int x) { m_maxMultiPartSize = x; }                  ///< Sets maximum request size in multipart requests

    const QString&  tempDir() const { return m_tempDir; }                                   ///< Returns path to temporary directory
    void            setTempDir(const QString& x) { m_tempDir = x; }                         ///< Sets path to temporary directory

    bool            persistent() const { return m_persistent ; }
    void            setPersistent(bool x) { m_persistent = x; }



  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    int             m_port;
    QHostAddress    m_address;
    int             m_timeout;
    int             m_maxAge;
    QString         m_encoding;
    QString         m_docroot;
    int             m_maxCachedFileSize;
    int             m_cacheTimeout;
    QString         m_indexFile;
    bool            m_cacheOn;
    QString         m_sessionCookieName;               ///< The name of session cookie
    int             m_sessionExpirationTime;           ///< Expiration age of session
    int             m_maxRequestSize;
    int             m_maxMultiPartSize;
    QString         m_tempDir;
    bool            m_persistent;
    #endif

    void            readSettings(const QSettings* settings, const QString& section);

};

}

#endif
