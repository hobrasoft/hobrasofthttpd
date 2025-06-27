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
#include "staticfilecontroller.h"
#include <QSet>
#include <QSetIterator>
#include <QHashIterator>

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
        response->setCookie(
                HttpCookie(
                    sessionCookieName(),
                    session.id(),
                    m_settings->sessionExpirationTime(),
                    "/"
                    )
                );
        response->setHeader(
                "X-Session-Expires", 
                StaticFileController::toGMTString(QDateTime::currentDateTime().addSecs(m_settings->sessionExpirationTime()).toUTC()) 
                );
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
    response->setHeader(
            "X-Session-Expires", 
            StaticFileController::toGMTString(QDateTime::currentDateTime().addSecs(m_settings->sessionExpirationTime()).toUTC()) 
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
    QSet<QByteArray> sessionsToRemove;
    QHashIterator<QByteArray, HttpSession> iterator(m_sessions);
    while (iterator.hasNext()) {
        iterator.next();
        const QByteArray& key = iterator.key();
        const HttpSession& session = iterator.value();
        if (!session.lastAccess().isValid() || session.lastAccess().addSecs(m_settings->sessionExpirationTime()) < now) {
            emit aboutToRemove(session);
            sessionsToRemove << key;
            }
        }

    QSetIterator<QByteArray> iteratorR(sessionsToRemove);
    while (iteratorR.hasNext()) {
        const QByteArray& key = iteratorR.next();
        m_sessions.remove(key);
        }
}


void HttpSessionStore::remove(HttpSession session) {
    emit aboutToRemove(session);
    m_sessions.remove(session.id());
}


void HttpSessionStore::setSessionData(const HttpSessionData& data) {
    HttpSession session(data);
    m_sessions.insert(session.id(), session);
}


