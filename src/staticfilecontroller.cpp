/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "staticfilecontroller.h"
#include "httpconnection.h"
#include "httpsettings.h"
#include "httpresponse.h"
#include "httprequest.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QRegExp>

using namespace HobrasoftHttpd;

QHash<QString, QString> StaticFileController::m_mimetypes;

StaticFileController::StaticFileController(HttpConnection *parent) : HttpRequestHandler(parent) {

    if (m_mimetypes.isEmpty()) { 
        addMimeType("png",   "image/png");
        addMimeType("jpeg",  "image/jpeg");
        addMimeType("jpg",   "image/jpeg");
        addMimeType("gif",   "image/gif");
        addMimeType("txt",   "text/plain");
        addMimeType("html",  "text/html");
        addMimeType("xhtml", "text/html");
        addMimeType("htm",   "text/html");
        addMimeType("css",   "text/css");
        addMimeType("json",  "application/json");
        addMimeType("js",    "application/javascript");
        }

    m_parent = parent;
}


const HttpSettings *StaticFileController::settings() const { 
    return m_parent->settings(); 
}


void StaticFileController::addMimeType(const QString& fileSuffix, const QString& mimetype) {
    m_mimetypes[fileSuffix] = mimetype;
}


void StaticFileController::service(HttpRequest *request, HttpResponse *response) {
    QString path = request->path();
    QString rootpath = QDir::fromNativeSeparators(settings()->docroot());

    if (path.startsWith("/..")) {
        response->setStatus(403,"Forbidden");
        response->write("403 Forbidden");
        response->flush();
        return;
        }

    QString filename = rootpath + path;
    if (QFileInfo(filename).isDir()) {
        filename += "/" + settings()->indexFile();
        }

    QFile file(QDir::toNativeSeparators(filename));
    if (!file.exists()) {
        response->setStatus(404, "Not found");
        response->write("404 Not found");
        response->flush();
        return;
        }

    if (!file.open(QIODevice::ReadOnly)) {
        response->setStatus(403, "Forbidden");
        response->write("403 Forbidden");
        response->flush();
        return;
        }

    setContentType(path, response);
    response->setHeader("Cache-Control", QString("max-age=") + QString("%1").arg(settings()->maxAge()) );
    response->write( file.readAll() );
    response->flush();
    file.close();
}


void StaticFileController::setContentType(const QString& filename, HttpResponse *response) const {
    QString koncovka = QString(filename).replace(QRegExp("^.*\\."), "").toLower();;
    if (koncovka.isEmpty()) return;
    if (!m_mimetypes.contains(koncovka)) return;

    QString type = m_mimetypes[koncovka];
    response->setHeader("Content-Type", type);

}

