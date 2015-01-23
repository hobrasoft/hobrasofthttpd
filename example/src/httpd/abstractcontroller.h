/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _Httpd_AbstractController_H_
#define _Httpd_AbstractController_H_

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include "httprequesthandler.h"

namespace HobrasoftHttpd {
class HttpRequestHandler;
class HttpRequest;
class HttpResponse;
class HttpConnection;
}

namespace Httpd {

/**
@brief Virtuální třída pro vyřizování unifikovaných Http požadavků na server API

Třída vytváří šablonu pro tvorbu unifikovaných HTTP požadavků podle specifikace api. Požadavky jsou
tvořené s pevnou strukturou, například:

~~~~~~~~~~~~~~
http://localhost:8086/room
http://localhost:8086/room/events
http://localhost:8086/room/e40f2af7ea281baba30381db700311f7
http://localhost:8086/room/e40f2af7ea281baba30381db700311f7/events
~~~~~~~~~~~~~~

Cesta je složená z několika komponent:

- /room - modul, kterého se týká. Pokud neuvedu další části cesty, vrací taková adresa okamžitý seznam všech objektů (zde pokojů)
- e40f2af7ea281baba30381db700311f7 - id objektu, který chci obsluhovat
- events - pokud se přidá na konec cesty slovo events, otevře se proud událostí, do kterého se začnou vypisovat všechny změny ve
      sledovaných objektech. Pokud je objektem celý seznam, vypisují se změny každého objektu v seznamu (zde změny na všech pokojích).
      Pokud je objektem konkrétní objekt, vypisují se pouze jeho změny (zde změny na konkrétním pokoji).

Pro obsluhu cest se volají různé metody:

- /room - serviceList()
- /room/events - serviceEvents()
- /room/e40f2af7ea281baba30381db700311f7 - podle HTTP metody požadavku se volá některá z metod 
        serviceIdGet(), serviceIdDelete(), serviceIdPut(), serviceIdPost().
- /room/e40f2af7ea281baba30381db700311f7/events - volá se metoda serviceEvent()

### Implementace odvozených tříd ###

Defaultní implementace vrací chybu 501. V odvozené třídě se musí implementovat všechny metody, které mají vracet užitečná data.
Může být důležité implementovat i metodu exists() - tato metoda vrací bool hodnotu true, pokud dotazované ID existuje.
Pokud metoda exists() vrátí false, defaultní implementace třídy vrátí jako odpověď na http požadavek chybu 404. Implementace
konkrétní metody se tak nemusí starat o odesílání chybových stavů, například:

~~~~~~~~~~~~~~
void ControllerRoom::serviceIdGet (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    Pokoj *pokoj = SEZNAMPOKOJU->pokoj(id);
    if (pokoj == NULL) {
        // Pokoj neexistuje, chybu už nahlásila abstraktní třída
        // Kontrola zde je zbytečná, ale je riskantní test neprovádět
        return;
        }
    serviceOK(request, response, pokoj->webStatus());
}
~~~~~~~~~~~~~~

Kontrola na existenci ID proběhla už v abstraktní třídě AbstractController, takže je v kódu zbytečná - k podobnému stavu by
nemělo nikdy dojít. Kontrola je zde pouze pro jistoru, aby server nespadnul při nepředvídatelných událostech. Dobrodružné
povahy mohou s velkou porcí jistoty zkrátit celou implementaci metody takto:

~~~~~~~~~~~~~~
void ControllerRoom::serviceIdGet (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    serviceOK(request, response, SEZNAMPOKOJU->pokoj(id)->webStatus());
}
~~~~~~~~~~~~~~

### Implementace proudu událostí ###
Implementace proudu událostí se dá zjednodušit použitím metody serviceEvent(). Tato metoda posílá v odpovědi předaná data
zformátovaná jako událost - postará se o správné zformátování a potlačí odeslání hlaviček. Implementace proudu událostí pak
může vypadat takto (pro stručnost jsou vyházené kontroly):

~~~~~~~~~~~~~~
void ControllerRoom::serviceEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    connect (SEZNAMPOKOJU, SIGNAL( statusChanged(const SkupinaJednotek *)),
             this,           SLOT(slotSendUpdate(const SkupinaJednotek *)));

    QList<SkupinaJednotek *> list = SEZNAMPOKOJU->skupiny().toList();
    for (int i=0; i<list.size(); i++) {
        slotSendUpdate(qobject_cast<Pokoj *>(list[i]));
        }
}

void ControllerRoom::serviceIdEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    connect(pokoj, SIGNAL(statusChanged(const SkupinaJednotek *)),
             this,   SLOT(slotSendUpdate(const SkupinaJednotek *)));

    slotSendUpdate(pokoj);
}

void ControllerRoom::slotSendUpdate(const SkupinaJednotek *skupina) {
    serviceEvent(NULL, NULL, pokoj->webStatusPresenceAndCall());
}
~~~~~~~~~~~~~~

@see @ref api

*/
class AbstractController : public HobrasoftHttpd::HttpRequestHandler {
    Q_OBJECT
  public:

    /**
     * @brief Konstruktor
     */
    AbstractController(HobrasoftHttpd::HttpConnection *parent);

    /**
     * @brief Obsluha požadavku
     */
    virtual void service(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

    /**
     * @brief Nastavuje kontrolu položky ID v datech při PUT a POST
     */
    void setCheckId(bool x) { m_checkId = x; }

    /**
     * @brief Vrací stav kontroly položky ID v datech při PUT a POST
     */
    bool checkId() const { return m_checkId; }

  protected:

    /**
     * @brief Kontrola existence zadaného ID
     * 
     * @returns true pokud zadané ID existuje
     *
     * V odvozených třídách by měla být metoda reimplementovaná. Na základě
     * výsledků se rozhoduje, jestli se pošle jako odpověď na dotaz chyba 404,
     * nebo se bude volat některá z dalších metod (serviceId(),
     * serviceIdEvents(), serviceIdDelete()).
     */
    virtual bool exists(const QString& id) const { Q_UNUSED(id); return true; }


    /**
     * @brief Pošle odpověď 200 OK
     *
     * @param data - pokud je zadaný, pošle se místo defaultního '{ "ok" : true }'
     */
    virtual void serviceOK        (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariant& data = QVariant() );

    /**
     * @brief Pošle chybovou odpověď
     *
     * @param code - chybový kód (například 404)
     * @param error - chybový textový kód
     * @param reason - vysvětlení chyby
     *
     * Posílá odpověď zformátovanou takto:
     *
     * ~~~~~~~~~~~~~~
       Content-Type: application/json
       Cache-Control: no-cache,public
      
       404 not-found
       { "error" : "not-found", "reason" : "Not found" }
       ~~~~~~~~~~~~~~
     */
    virtual void serviceError     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, int code, const QString& error, const QString& reason);

    /**
     * @brief Měl by poslat v odpovědi kompletní seznam všech objektů ve třídě a ukončit se
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceList (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

    /**
     * @brief Pošle v odpovědi kompletní seznam všech objektů ve třídě a následně
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

    /**
     * @brief Pošle v odpovědi proud událostí spojených se všemi objekty ve třídě
     *
     * Obvyklá implementace pošle na začátku seznam všech objektů (jako události) a potom posílá už jen změny
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceIdEvents  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

    /**
     * @brief Pošle odpověď na požadavek konkrétního ID metodou GET
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceIdGet     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

    /**
     * @brief Pošle odpověď na požadavek konkrétního ID metodou DELETE - obvykle objekt smaže
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceIdDelete  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

    /**
     * @brief Pošle odpověď na požadavek konkrétního ID metodou PUT - obvykle vloží nový objekt nebo přepíše existující objekt
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceIdPut     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariantMap& data);

    /**
     * @brief Pošle odpověď na požadavek konkrétního ID metodou PUT - obvykle vloží nový objekt nebo přepíše existující objekt
     *
     * Defaultní implementace posílá chybu 501 Not Implemented
     */
    virtual void serviceIdPost    (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariantMap& data);

    /**
     * @brief Pošle do otevřeného proudu událostí jednu událost
     *
     * I když je možné reimplementovat, obvykle pro to není důvod. Posílá do proudu událostí jednu událost.
     */

    virtual void serviceEvent (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariant& data);

    HobrasoftHttpd::HttpRequest    *request() const;        ///< Vrací ukazatel na aktuální request
    HobrasoftHttpd::HttpConnection *connection() const;     ///< Vrací ukazatel na aktuální connection

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    HobrasoftHttpd::HttpRequest    *m_request;
    HobrasoftHttpd::HttpConnection *m_connection;
    bool    m_checkId;
    #endif

};

}

#endif
