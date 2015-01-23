#ifndef _List_H_
#define _List_H_

#include <QObject>
#include <QHash>
#include <QVariantList>

class Item;

class List : public QObject {
    Q_OBJECT
  public:
    List(QObject *parent);

    QVariantList webStatus() const;

    void insert(Item *);

    bool contains(const QString& id) const;

    Item *item(const QString& id) const;

  signals:
    void    statusChanged(const Item *);

  private:
    QHash<QString, Item *>  m_items;
    

};

#endif
