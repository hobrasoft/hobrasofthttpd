/**
 * @file
 *
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#include <QTextCodec>
#include <QCoreApplication>
#include "application.h"
#include "httpd.h"
#include "main.h"

Example::Application *Example::application = NULL;

int main (int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    app.setOrganizationName   ("hobrasoft");
    app.setOrganizationDomain ("hobrasoft.cz");
    app.setApplicationName    ("hobrasofthttpd");
    app.setApplicationVersion ("1.0.0");

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    Example::application = new Example::Application(0);
    new Example::Httpd::Httpd(0);

    return app.exec();

}



