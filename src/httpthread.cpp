/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */
#include "httpthread.h"
#include <QThread>
#include <signal.h>

namespace HobrasoftHttpd {

void HttpThread::run() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    sigemptyset(&set);
    sigaddset(&set, SIGKILL);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    QThread::run();
};

}

