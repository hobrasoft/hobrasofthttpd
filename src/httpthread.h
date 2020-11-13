/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpThread_H_
#define _HttpThread_H_

#include <QThread>

namespace HobrasoftHttpd {

class HttpThread : public QThread {
    Q_OBJECT
  public:
    HttpThread(QObject *parent) : QThread(parent) {}

  protected:
    void run() Q_DECL_OVERRIDE;

};

}

#endif
