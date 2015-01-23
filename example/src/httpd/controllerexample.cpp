/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "controllerexample.h"
#include "application.h"
#include "list.h"
#include "item.h"
#include "main.h"

#define LIST application->list()

using namespace Httpd;


void ControllerExample::serviceList (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    serviceOK(request, response, LIST->webStatus());
}


bool ControllerExample::exists (const QString& id) const {
    return LIST->contains(id);
}


void ControllerExample::serviceEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
    Q_UNUSED(request);
    Q_UNUSED(response);
    connect (LIST, SIGNAL( statusChanged(const Item *)),
             this,   SLOT(    sendUpdate(const Item *)));

/*
    QList<SkupinaJednotek *> list = SEZNAMPOKOJU->skupiny().toList();
    for (int i=0; i<list.size(); i++) {
        slotSendUpdate(qobject_cast<Pokoj *>(list[i]));
        }
*/
}


void ControllerExample::serviceIdEvents (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    Q_UNUSED(request);
    Q_UNUSED(response);

    Item *item = LIST->item(id);

    connect(item, SIGNAL(statusChanged(const Item *)),
            this,   SLOT(   sendUpdate(const Item *)));

    sendUpdate(item);
}


void ControllerExample::serviceIdGet (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
    serviceOK(request, response, LIST->item(id)->webStatus());
}


void ControllerExample::sendUpdate(const Item *item) {
    serviceEvent(NULL, NULL, item->webStatus());
}



