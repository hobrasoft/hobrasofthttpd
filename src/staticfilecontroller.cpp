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
        addMimeType("shtml", "text/html");
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
    QFileInfo fileinfo(filename);
    if (fileinfo.isDir()) {
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

    QString suffix = fileinfo.suffix();
    if (!suffix.isEmpty() && m_mimetypes.contains(suffix)) {
        response->setHeader("Content-Type", m_mimetypes[suffix]);
        }

    response->setHeader("Cache-Control", QString("Public,max-age=") + QString("%1").arg(settings()->maxAge()) );
    response->setHeader("Expires", toGMTString(QDateTime::currentDateTime().addSecs(settings()->maxAge()).toUTC()) );
    response->write( file.readAll() );
    response->flush();
}


QString StaticFileController::toGMTString(const QDateTime& x) {
    QString dayname;
    switch (x.date().dayOfWeek()) {
        case 1: dayname = "Mon"; break;
        case 2: dayname = "Tue"; break;
        case 3: dayname = "Wed"; break;
        case 4: dayname = "Thu"; break;
        case 5: dayname = "Fri"; break;
        case 6: dayname = "Sat"; break;
        case 7: dayname = "Sun"; break;
        }

    QString monthname;
    switch (x.date().month()) {
        case  1: monthname = "Jan"; break;
        case  2: monthname = "Feb"; break;
        case  3: monthname = "Mar"; break;
        case  4: monthname = "Apr"; break;
        case  5: monthname = "May"; break;
        case  6: monthname = "Jun"; break;
        case  7: monthname = "Jul"; break;
        case  8: monthname = "Aug"; break;
        case  9: monthname = "Sep"; break;
        case 10: monthname = "Oct"; break;
        case 11: monthname = "Nov"; break;
        case 12: monthname = "Dec"; break;
        }

    QString string = QString("%1, %2 %3 %4 %5:%6:%7 GMT")
                        .arg(dayname)
                        .arg(x.date().day())
                        .arg(monthname)
                        .arg(x.date().year())
                        .arg(x.time().hour())
                        .arg(x.time().minute())
                        .arg(x.time().second())
                        ;
    return string;
}


