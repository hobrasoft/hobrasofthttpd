#ifndef _Application_H_
#define _Application_H_

#include <QObject>
#include "list.h"


class Application : public QObject {
    Q_OBJECT
  public:
    Application(QObject *);

    List *list() const;

  private:
    List *m_list;
};

#endif

