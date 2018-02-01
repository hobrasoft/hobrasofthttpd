/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "httpsessionstore.h"
#include "httpsession.h"
#include "httpresponse.h"
#include "httprequest.h"
#include "httpsettings.h"
#include "httpserver.h"

using namespace HobrasoftHttpd;


HttpSessionStore::HttpSessionStore(const HttpSettings* settings, QObject *parent) : QObject(parent) {
    m_settings = settings;
    init();
}


HttpSessionStore::HttpSessionStore(HttpServer *server) : QObject(server) {
    m_settings = server->settings();
    init();
}


void HttpSessionStore::init() {
    m_cleaner = new QTimer(this);
    m_cleaner->setInterval(30000);
    m_cleaner->setSingleShot(false);
    m_cleaner->start();
    connect(m_cleaner, SIGNAL(timeout()), this, SLOT(slotCleaner()));
}


const QString& HttpSessionStore::sessionCookieName() const { 
    if (m_sessionCookieName.isEmpty() || m_sessionCookieName == "") {
        return m_settings->sessionCookieName();
        }
    return m_sessionCookieName; 
}


QByteArray HttpSessionStore::sessionId(HttpRequest *request, HttpResponse *response) {
    QByteArray sessionId = response->cookie(sessionCookieName()).value().toUtf8();
    if (sessionId.isEmpty()) {
        sessionId = request->cookie(sessionCookieName()).toUtf8();
        }

    if (sessionId.isEmpty() || !m_sessions.contains(sessionId)) {
        return QByteArray();
        }

    return sessionId;
}


HttpSession HttpSessionStore::session(HttpRequest *request, HttpResponse *response) {
    QByteArray id = sessionId(request, response);
    if (!id.isEmpty()) {
        HttpSession session(m_sessions.value(id));
        session.setLastAccess();
        return session;
        }

    HttpSession session;
    m_sessions.insert(session.id(), session);
    response->setCookie(
            HttpCookie(
                sessionCookieName(),
                session.id(),
                m_settings->sessionExpirationTime(),
                "/"
                )
            );
    return session;
}


HttpSession HttpSessionStore::session(const QByteArray& id) {
    HttpSession session = m_sessions.value(id, HttpSession());
    session.setLastAccess();
    return session;
}


void HttpSessionStore::slotCleaner() {
    QDateTime now = QDateTime::currentDateTime();
    QHash<QByteArray, HttpSession>::iterator i = m_sessions.begin() ;
    while (i != m_sessions.end()) {
        QHash<QByteArray, HttpSession>::iterator prev = i;
        i++;
        HttpSession session = prev.value();
        if (!session.lastAccess().isValid() || session.lastAccess().addSecs(m_settings->sessionExpirationTime()) < now) {
            emit aboutToRemove(session);
            m_sessions.erase(prev);
            }
        }
}


void HttpSessionStore::remove(HttpSession session) {
    emit aboutToRemove(session);
    m_sessions.remove(session.id());
    session.freeData();
}


