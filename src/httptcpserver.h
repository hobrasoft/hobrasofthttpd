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
#include <QSslCertificate>
#include <QPointer>
#include "httpconnection.h"

namespace HobrasoftHttpd {

class HttpServer;
class HttpSettings;

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
    void    newConnection(QPointer<HttpConnection>);

  private:
    void    incomingConnection(QINTPTR socketDescriptor);

    const HttpSettings   *m_settings;
    HttpServer *m_parent;

};

}

#endif

