/**
 * @file
 * 
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _HttpTcpServer_H_
#define _HttpTcpServer_H_

#include <QTcpServer>
#include <QList>
#include <QSslError>

namespace HobrasoftHttpd {

class HttpServer;
class HttpSettings;

/**
 * @brief
 */
class HttpTcpServer : public QTcpServer {
    Q_OBJECT
  public:
    HttpTcpServer(HttpServer *);

  signals:

  private slots:
    void    slotEncrypted();
    void    slotSslErrors(const QList<QSslError>&);

  private:
    void    incomingConnection(int socketDescriptor);

    const HttpSettings   *m_settings;

};

}

#endif

