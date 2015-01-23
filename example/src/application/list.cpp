#include "list.h"
#include "item.h"

List::List(QObject *parent) : QObject(parent) {
}


QVariantList List::webStatus() const {
    QVariantList data;
    QHashIterator<QString, Item *> iterator(m_items);
    while (iterator.hasNext()) {
        iterator.next();
        Item *item = iterator.value();
        data << item->webStatus();
        }
    return data;
}


void List::insert(Item *item) {
    m_items[item->id()] = item;
    connect (item, SIGNAL(statusChanged(const Item *)), this, SIGNAL(statusChanged(const Item *)));
}


bool List::contains(const QString& id) const {
    return m_items.contains(id);
}


Item *List::item(const QString& id) const {
    return m_items.value(id, NULL);
}


