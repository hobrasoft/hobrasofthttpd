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

namespace Example {
namespace Httpd {

/**
@brief Abstract class for processing unified HTTP requests to server's API

Data are passed in QVariant structures and converted to/from JSON for HTTP transport.
The data can be created like this:
~~~~~~~~~~~~~~
QVariantMap data;
data["ID"] = "1";
data["Description"] = "Some text";
QVariantList list;
list << "abc" << "xyz";
data["List"] = list;
// corresponding json: { "ID" : "1", "Description" : "Some text", "List" : [ "abc", "xyz" ] }
~~~~~~~~~~~~~~


Class is a virtual template for unified HTTP requests. Requests are created with
stable structure, for example the list of rooms:

~~~~~~~~~~~~~~
http://localhost:8086/room
http://localhost:8086/room/events
http://localhost:8086/room/e40f2af7ea281baba30381db700311f7
http://localhost:8086/room/e40f2af7ea281baba30381db700311f7/events
~~~~~~~~~~~~~~

The path consist of few components:
- /room - module from which the data are acquired. It no other part of the path is given, such URL returns list of all objects in the modules.
- e40f2af7ea281baba30381db700311f7 - object id
- events - if the world _events_ is added to the end of the path, then the event stream is created. All changes in module
   is written to the stream. There are two cases: if the id was givent, then only events associated with the id are sent, otherwise
   all event for every object in the modules is sent.

Different methods are called to retrieve data:

- /room - serviceList()
- /room/events - serviceEvents()
- /room/e40f2af7ea281baba30381db700311f7 - depending on HTTP method some of these methods is called:
        serviceIdGet(), serviceIdDelete(), serviceIdPut(), serviceIdPost().
- /room/e40f2af7ea281baba30381db700311f7/events - the serviceEvent() method is called

### Derived class implementation ###

Default implementation returns error 501. In the derived class all method should be implemeted, which returns some useful data.
It can be important to implement also the exists() method - it returns true if the requested ID exists.
If it returns false, then the default AbstractController method returned error 404 before your own implementation is called
and the connection is closed.
Your own implementation can avoid all the care about error states:

~~~~~~~~~~~~~~
void ControllerRoom::serviceIdGet (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    // It is useles to find if the room exists, abstract class cares about it
    serviceOK(request, response, ROOMSLIST->room(id))
}
~~~~~~~~~~~~~~


### HTML5 event stream implementation ###
The HTML5 event stream can be simply implemented in serviceEvent() method. The method sends responses formated as an event - the
data are formated properly and the HTTP headers are supressed.
Implementation of event stream can look like this:

~~~~~~~~~~~~~~
void ControllerRoom::serviceEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    connect (ROOMSLIST, SIGNAL( statusChanged(const Room *)),
             this,        SLOT(slotSendUpdate(const Room *)));

    QList<Room *> list = ROMSLIST->rooms().toList();
    for (int i=0; i<list.size(); i++) {
        slotSendUpdate(list[i]);
        }
}

void ControllerRoom::serviceIdEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    connect(room, SIGNAL( statusChanged(const Room *)),
            this,   SLOT(slotSendUpdate(const Room *)));

    slotSendUpdate(room);
}

void ControllerRoom::slotSendUpdate(const Room *skupina) {
    serviceEvent(NULL, NULL, room->webStatus());
}
~~~~~~~~~~~~~~

Data are passed in QVariant structures and formated in JSON to be send to clients.

*/
class AbstractController : public HobrasoftHttpd::HttpRequestHandler {
    Q_OBJECT
  public:

    /**
     * @brief Constructor
     */
    AbstractController(HobrasoftHttpd::HttpConnection *parent);

    /**
     * @brief Request processing
     */
    virtual void service(HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

    /**
     * @brief If the checkId is set then the exixtence of the item is checked in requests PUT and POST
     */
    void setCheckId(bool x) { m_checkId = x; }

    /**
     * @brief Returns the state of checkId
     *
     * @see setCheckId()
     */
    bool checkId() const { return m_checkId; }

  protected:

    /**
     * @brief Check existence of th ID
     * 
     * @returns true if the ID exists 
     *
     * The method should be reimplemented in derived classes. Depending of the
     * result the decission is made if the 404 error is sent to the request, 
     * of if the reimplemented method will be called 
     * (one of serviceId(), serviceIdEvents(), serviceIdDelete()).
     */
    virtual bool exists(const QString& id) const { Q_UNUSED(id); return true; }


    /**
     * @brief Sends 200 OK response
     *
     * @param data - data are send instead of default '{ "ok" : true }'
     */
    virtual void serviceOK (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariant& data = QVariant() );

    /**
     * @brief Send an error response
     *
     * @param code - error code (404)
     * @param error - error brief description (not-found) - this is send also in the HTTP response header
     * @param reason - detailed description of the error
     *
     * Sends the response formatted like this:
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
     * @brief Sends list of all object in the module
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceList (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

    /**
     * @brief Sends list of all object in the module and then sends all changes in the module as an event stream
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response);

    /**
     * @brief Sends an event stream associated with the given ID
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceIdEvents  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

    /**
     * @brief Sends the object with given ID
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceIdGet     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

    /**
     * @brief Makes the DELETE operation with given ID
     *
     * Usualy the object is deleted in the application.
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceIdDelete  (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id);

    /**
     * @brief Makes the PUT operation with given ID
     *
     * Usualy the object is rewriten in the application.
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceIdPut     (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariantMap& data);

    /**
     * @brief Makes the POST operation with given ID
     *
     * Usualy the object is rewriten in the application.
     *
     * Default implementation returns error 501 Not Implemented
     */
    virtual void serviceIdPost    (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariantMap& data);

    /**
     * @brief Send one event to opened event stream
     *
     * Usualy it is not needed to reimplement this method.
     */

    virtual void serviceEvent (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QVariant& data);

    HobrasoftHttpd::HttpRequest    *request() const;        ///< Returns pointer to current request
    HobrasoftHttpd::HttpConnection *connection() const;     ///< Returns pointer to current connection

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    HobrasoftHttpd::HttpRequest    *m_request;
    HobrasoftHttpd::HttpConnection *m_connection;
    bool    m_checkId;
    #endif

};

}
}

#endif
