#include "application.h"
#include "list.h"
#include "item.h"


Application::Application(QObject *parent) : QObject(parent) {
    m_list = new List(this);
    list()->insert( new Item("A", "Item A",  987, this) );
    list()->insert( new Item("B", "Item B", 1597, this) );
    list()->insert( new Item("C", "Item C", 2584, this) );
    list()->insert( new Item("D", "Item D", 4181, this) );
}


List *Application::list() const {
    return m_list;
}
