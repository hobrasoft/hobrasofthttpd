/**
 * @file
 * 
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _HttpTcpServer_H_
#define _HttpTcpServer_H_

#include <QTcpServer>
#include <QList>
#include <QHash>
#include <QSslError>
#include <QSslCertificate>

namespace HobrasoftHttpd {

class HttpServer;
class HttpSettings;

#if QT_VERSION > 0x050000
#define QINTPTR qintptr
#else
#define QINTPTR int
#endif

/**
 * @brief Listens for incoming TCP connections, supports plain and ssl connections
 *
 * Class stores information of each connection - 
 * its verification status and peer's certificate.
 */
class HttpTcpServer : public QTcpServer {
    Q_OBJECT
  public:
    HttpTcpServer(HttpServer *);

    bool verified(QTcpSocket *) const;

    QSslCertificate peerCertificate(QTcpSocket *) const;

  signals:

  private slots:
    void    slotEncrypted();
    void    slotSslErrors(const QList<QSslError>&);
    void    slotPeerVerifyError(const QSslError&);
    void    slotDisconnected();

  private:
    void    incomingConnection(QINTPTR socketDescriptor);

    const HttpSettings   *m_settings;

    /**
     * @brief Verified status of each socket
     */
    QHash<QTcpSocket *, bool>   m_verified;

    /**
     * @brief Peer's certificate of each socket
     */
    QHash<QTcpSocket *, QSslCertificate>   m_peerCert;

};

}

#endif

