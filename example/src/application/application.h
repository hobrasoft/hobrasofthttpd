/**
 * @file
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _Application_H_
#define _Application_H_

#include <QObject>
#include "list.h"

namespace Example {

/**
 * @brief Example application
 */
class Application : public QObject {
    Q_OBJECT
  public:
    /**
     * @brief Constructor - makes the list and add some items to it
     */
    Application(QObject *);

    List *list() const; ///< Returns pointer to list of all items

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    List *m_list;
    #endif
};
}

#endif

