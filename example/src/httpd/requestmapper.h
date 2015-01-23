/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _RequestMapper_H_
#define _RequestMapper_H_

#include <QObject>
#include "httprequesthandler.h"

namespace HobrasoftHttpd {
class HttpRequestHandler;
class HttpRequest;
class HttpResponse;
class HttpConnection;
}

namespace Httpd {

/**
 * @brief Zpracovává HTTP požadavky, vyřizuje speciální požadavky
 *
 * Statické soubory a shtml stačí zpracovávat defaultním handlerem
 * HobrasoftHttpd::HttpRequestHandler, ale požadavky dotazující
 * vnitřní stav aplikace musejí být naprogramované v C++
 *
 */
class RequestMapper : public HobrasoftHttpd::HttpRequestHandler {
    Q_OBJECT
  public:

   ~RequestMapper();

    /**
     * @brief Konstruktor
     */
    RequestMapper(HobrasoftHttpd::HttpConnection *parent);

    /**
     * @brief Vyřizuje jeden požadavek
     *
     * Rozděluje podle typu požadavku do specializovaných handlerů 
     */
    void service(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

  private:
    QString     m_user;      ///< Uživatelské jméno pro přihlášení, bere se z konfigurace
    QString     m_password;  ///< Uživatelské heslo pro přihlášení, bere se z konfigurace 
    bool        m_authorize; ///< Jestli se má přihlašovat, bere se z konfigurace
    QString     m_path;      ///< Obsluhovaná cesta (URL adresa)


};

}

#endif
