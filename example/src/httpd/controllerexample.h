/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _Httpd_ControllerExample_H_
#define _Httpd_ControllerExample_H_

#include <QObject>
#include "abstractcontroller.h"


namespace Example {
class Item;

namespace Httpd {

/**
 * @brief Processes requests with path /example
 */
class ControllerExample : public AbstractController {
    Q_OBJECT
  public:
    ControllerExample(HobrasoftHttpd::HttpConnection *parent) : AbstractController(parent) {} 

  protected:

    virtual bool exists (const QString& id) const;

    virtual void serviceList      (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    virtual void serviceEvents    (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);
    virtual void serviceIdEvents  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);
    virtual void serviceIdGet     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

  private slots:
    /**
     * @brief Sends data of the item to the event stream
     */
    void    sendUpdate(const Item *);

};

}
}

#endif
