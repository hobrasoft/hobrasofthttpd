/**
 * @file
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _List_H_
#define _List_H_

#include <QObject>
#include <QHash>
#include <QVariantList>

namespace Example {

class Item;

/**
 * @brief List of all items
 */
class List : public QObject {
    Q_OBJECT
  public:
    List(QObject *parent);

    QVariantList webStatus() const;

    /**
     * @brief Insert one item to the list
     */
    void insert(Item *);

    /**
     * @brief Returns true if item with the given id exists
     */
    bool contains(const QString& id) const;

    /**
     * @brief Returns pointer to an item with given id or NULL if the item does not exist
     */
    Item *item(const QString& id) const;

  signals:
    /**
     * @brief Signal is invoked when some item status changed
     */
    void    statusChanged(const Item *);

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    QHash<QString, Item *>  m_items;    
    #endif
    

};

}

#endif
