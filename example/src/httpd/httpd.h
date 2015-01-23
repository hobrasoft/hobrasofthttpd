/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _Httpd_H_
#define _Httpd_H_

#include <QObject>
#include "httpserver.h"

namespace HobrasoftHttpd {
class HttpServer;
class HttpConnection;
class HttpRequestHandler;
}

/**
 * @brief Jmenný prostor pro obsluhu konkrétních HTTP požadavků aplikace
 */
namespace Httpd {


/**
 * @brief Konkrétní implementace HTTP serveru, rozšiřuje obecnou třídu HobrasoftHttpd::HttpServer, singleton
 *
 * Pro rozšíření je potřeba přepsat virtuální metodu requestHandler()
 */
class Httpd : public HobrasoftHttpd::HttpServer {
    Q_OBJECT
  public:
    Httpd(QObject *parent);

    virtual HobrasoftHttpd::HttpRequestHandler *requestHandler(HobrasoftHttpd::HttpConnection *);

  private:

};

}

#endif
