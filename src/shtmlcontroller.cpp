/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include "shtmlcontroller.h"
#include "httpconnection.h"
#include "httpresponse.h"
#include "httprequest.h"
#include "httpsettings.h"
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>

using namespace HobrasoftHttpd;

ShtmlController::ShtmlController(HttpConnection *parent) : HttpRequestHandler(parent) {
    m_parent = parent;
}


void ShtmlController::service(HttpRequest *request, HttpResponse *response) {
    QString path = request->path();

    QByteArray data;
    try {
        data = readFile(path, response, 0);
        } 
    catch (...) {
        return;
        }
    response->setHeader("Content-Type", "text/html; charset=" + settings()->encoding());
    response->write(data);
    response->flush();

}


QByteArray ShtmlController::readFile(const QString& path, HttpResponse *response, int depth) {
    QString rootpath = QDir::fromNativeSeparators(settings()->docroot());
    depth += 1;
    if (depth > 10) {
        response->setStatus(500,"Server error");
        response->write(QString("500 server error: %1<br>\nRecursive loop detected in included files.").arg(path).toUtf8());
        response->flush();
        throw false;;
        }

    QByteArray data;
    if (path.startsWith("/..") || path.startsWith("..")) {
        response->setStatus(403,"Forbidden");
        response->write(QString("403 Forbidden: %1<br>\nDo not use ../ in your file path").arg(path).toUtf8());
        response->flush();
        throw false;;
        }

    QString filename = rootpath + "/" + path;
    QFile file(filename);

    if (QFileInfo(filename).isDir()) {
        response->setStatus(500, "Server error");
        response->write(QString("500 Server error: %1<br>\nCannot include directory").arg(file.fileName()).toUtf8());
        response->flush();
        throw false;
        }

    if (!file.exists()) {
        response->setStatus(404, "Not found");
        response->write(QString("404 File not found: %1").arg(file.fileName()).toUtf8());
        response->flush();
        throw false;;
        }

    if (!file.open(QIODevice::ReadOnly)) {
        response->setStatus(403, "Forbidden");
        response->write(QString("403 Forbidden: %1").arg(file.fileName()).toUtf8());
        response->flush();
        throw false;;
        }

    while (!file.atEnd()) {
        QString line = QString::fromUtf8(file.readLine());
        if (line.contains(QRegExp("^\\s*<!--\\s*#include\\s+\".+\"\\s*-->\\s*$"))) {
            QStringList lineparts = line.split('"');
            if (lineparts.size() != 3) {
                continue;
                }
            data += readFile(lineparts[1], response, depth);
            continue;
            }
        data += line.toUtf8();
        }

    file.close();
    return data;
}

