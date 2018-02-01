/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpsession.h"
#include <QUuid>
#include <QDebug>

using namespace HobrasoftHttpd;



HttpSession::HttpSession() {
    m_data = new HttpSessionData();
    m_data->lastAccess = QDateTime::currentDateTime();
    m_data->id = QUuid::createUuid().toString().toUtf8();
}


HttpSession::HttpSession(const HttpSession& other) {
    QMutexLocker locker1(&m_mutex);
    m_data = other.m_data;
    if (m_data != NULL) {
        m_data->lastAccess = QDateTime::currentDateTime();
        }
}


HttpSession& HttpSession::operator= (const HttpSession& other) {
    QMutexLocker locker1(&m_mutex);
    m_data = other.m_data;
    if (m_data != NULL) {
        m_data->lastAccess = QDateTime::currentDateTime();
        }
    return *this;
}

