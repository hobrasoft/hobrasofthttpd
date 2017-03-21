/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HTTP_SETTINGS_H_
#define _HTTP_SETTINGS_H_

#include <QHostAddress>
#include <QSettings>
#include <QSslError>
#include <QSet>

int qHash(const QSslError error);

namespace HobrasoftHttpd {

/**
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
timeout = 60000
@endcode

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
    void            setDefaultAddress(const QHostAddress& x) { m_default_address = x; }     ///< Sets the defailt bind address

    int             port() const { return m_port; }                                         ///< Returns the port number 
    void            setPort(int x) { m_port = x; }                                          ///< Sets the port number
    void            setDefaultPort(int x) { m_default_port = x; }                           ///< Sets the default port number

    int             timeout() const { return m_timeout; }                                   ///< Returns timeout for opened connection from clients 
    void            setTimeout(int x) { m_timeout = x; }                                    ///< Sets timeout for opened connection from clients
    void            setDefaultTimeout(int x) { m_default_timeout = x; }                     ///< Sets default timeout for opened connection from clients

    int             maxAge() const { return m_maxAge; }                                     ///< Returns the max age for cacheing proxy  objects 
    void            setMaxAge(int x) { m_maxAge = x; }                                      ///< Sets the max age cacheing proxy  objects
    void            setDefaultMaxAge(int x) { m_default_maxAge = x; }                       ///< Sets the default max age cacheing proxy  objects

    const QString&  encoding() const { return m_encoding; }                                 ///< Returns the encoding in COntent-type header
    void            setEncoding(const QString& x) { m_encoding = x; }                       ///< Sets the encoding in COntent-type header
    void            setDefaultEncoding(const QString& x) { m_default_encoding = x; }        ///< Sets the default encoding in COntent-type header

    const QString&  docroot() const { return m_docroot; }                                   ///< Returns path to document root 
    void            setDocroot(const QString& x) { m_docroot = x; }                         ///< Sets path to document root
    void            setDefaultDocroot(const QString& x) { m_default_docroot = x; }          ///< Sets default path to document root

    const QString&  indexFile() const { return m_indexFile; }                               ///< Returns name of index file in directory (index.html) 
    void            setIndexFile(const QString& x) { m_indexFile = x; }                     ///< Sets name of index file in directory (index.html)
    void            setDefaultIndexFile(const QString& x) { m_default_indexFile = x; }      ///< Sets default name of index file in directory (index.html)

    int             sessionExpirationTime() const { return m_sessionExpirationTime; }       ///< Returns maximum expiration time for the session
    void            setExpirationTime(int x) { m_sessionExpirationTime = x; }               ///< Sets maximum expiration time for the session
    void            setDefaultExpirationTime(int x) { m_default_sessionExpirationTime = x;} ///< Sets default maximum expiration time for the session

    const QString&  sessionCookieName() const { return m_sessionCookieName; }               ///< Returns session cookie name 
    void            setSessionCookieName(const QString& x) { m_sessionCookieName = x; }     ///< Sets default session cookie name
    void            setDefaultSessionCookieName(const QString& x) { m_default_sessionCookieName = x; }     ///< Sets session cookie name

    int             maxRequestSize() const { return m_maxRequestSize; }                     ///< Returns maximum request size 
    void            setMaxRequestSize(int x) { m_maxRequestSize = x; }                      ///< Sets maximum request size
    void            setDefaultMaxRequestSize(int x) { m_default_maxRequestSize = x; }       ///< Sets default maximum request size

    int             maxMultiPartSize() const { return m_maxMultiPartSize; }                 ///< Returns maximum request size in multipart requests 
    void            setMaxMultiPartSize(int x) { m_maxMultiPartSize = x; }                  ///< Sets maximum request size in multipart requests
    void            setDefaultMaxMultiPartSize(int x) { m_default_maxMultiPartSize = x; }   ///< Sets default maximum request size in multipart requests

    const QString&  tempDir() const { return m_tempDir; }                                   ///< Returns path to temporary directory
    void            setTempDir(const QString& x) { m_tempDir = x; }                         ///< Sets path to temporary directory
    void            setDefaultTempDir(const QString& x) { m_default_tempDir = x; }          ///< Sets default path to temporary directory

    bool            useSSL() const { return m_useSSL; }                                     ///< Returns status of SSL connections
    void            setUseSSL(bool x) { m_useSSL = x; }                                     ///< Set status of SSL connections
    void            setDefaultUseSSL(bool x) { m_default_useSSL = x; }                      ///< Set default status of SSL connections

    bool            threads() const { return m_threads; }                                   ///< Returns true if server starts new thread for each connection
    void            setThreads(bool x) { m_threads = x; }                                   ///< Set starting new thread for each connection
    void            setDefaultThreads(bool x) { m_default_threads = x; }                    ///< Set default value for starting new thread for each connection

    const QString&  sslKey() const { return m_sslKey; }                                     ///< Returns SSL key
    void            setSslKey(const QString& x) { m_sslKey = x; }                           ///< Set SSL key
    void            setDefaultSslKey(const QString& x) { m_default_sslKey = x; }            ///< Set default SSL key

    const QString&  sslCrt() const { return m_sslCrt; }                                     ///< Returns SSL certificate
    void            setSslCrt(const QString& x) { m_sslCrt = x; }                           ///< Set SSL certificate
    void            setDefaultSslCrt(const QString& x) { m_default_sslCrt = x; }            ///< Set default SSL certificate

    const QString&  sslCaCrt() const { return m_sslCaCrt; }                                 ///< Returns SSL CA certificate
    void            setSslCaCrt(const QString& x) { m_sslCaCrt = x; }                       ///< Set SSL CA certificate
    void            setDefaultSslCaCrt(const QString& x) { m_default_sslCaCrt = x; }        ///< Set default SSL CA certificate


    bool            ignoreSslError(QSslError error) const;                                  ///< Returns true if the error should be ignored, default true

    /**
     * @brief Read settings from configuration file. 
     *
     * First are read values from first section (second parameter), then from section2 (third parameter).
     * From the second section are read values which were not read from first section.
     * You can have set your configuration file like this:

        ~~~~~~~~~~
        [httpd]
        address  = ::
        port     = 8086
        root     = /usr/share/appdata/htdocs
        user     = admin
        password = admin

        [httpds]
        port     = 8087
        sslKey   = /etc/ssl/private/server.hobrasoft.eu.key.pem
        sslCrt   = /etc/ssl/private/server.hobrasoft.eu.crt.pem
        useSSL   = true
        ~~~~~~~~~~

     */
    void    readSettings(const QSettings* settings, const QString& section, const QString& section2);


    /**
     * @brief Read settings from configuration file. 
     */
    void    readSettings(const QSettings* settings, const QString& section);

    /**
     * @brief Sets the name for the second section
     *
     * From the second section are read values which were not read from first section.
     */
    void            setSection2(const QString& section) { m_section2 = section; }

  protected:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    QString         m_section2;
    int             m_port;
    QHostAddress    m_address;
    int             m_timeout;
    int             m_maxAge;
    QString         m_encoding;
    QString         m_docroot;
    QString         m_indexFile;
    QString         m_sessionCookieName;               ///< The name of session cookie
    int             m_sessionExpirationTime;           ///< Expiration age of session
    int             m_maxRequestSize;
    int             m_maxMultiPartSize;
    QString         m_tempDir;
    bool            m_useSSL;
    QString         m_sslKey;
    QString         m_sslCrt;
    QString         m_sslCaCrt;
    QSet<QSslError> m_sslErrors;
    bool            m_ignoreAllSslErrors;
    bool            m_threads;

    // Default values
    QString         m_default_section2;
    int             m_default_port;
    QHostAddress    m_default_address;
    int             m_default_timeout;
    int             m_default_maxAge;
    QString         m_default_encoding;
    QString         m_default_docroot;
    QString         m_default_indexFile;
    QString         m_default_sessionCookieName;
    int             m_default_sessionExpirationTime;
    int             m_default_maxRequestSize;
    int             m_default_maxMultiPartSize;
    QString         m_default_tempDir;
    bool            m_default_useSSL;
    QString         m_default_sslKey;
    QString         m_default_sslCrt;
    QString         m_default_sslCaCrt;
    bool            m_default_ignoreAllSslErrors;
    bool            m_default_threads;
    #endif

  private:
    void            setDefaultValues();

};

}

#endif
