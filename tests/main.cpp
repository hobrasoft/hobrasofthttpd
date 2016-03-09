/**
 * @file
 *
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "test.h"
#include <QCoreApplication>

int main (int argc, char **argv) {
    QCoreApplication app(argc, argv);

    app.setOrganizationName   ("hobrasoft.cz");
    app.setOrganizationDomain ("hobrasoft.cz");
    app.setApplicationName    ("test-httpd");
    app.setApplicationVersion ("1.0.0");

    Test test;
    app.exit(QTest::qExec(&test));

    return 0;
}


