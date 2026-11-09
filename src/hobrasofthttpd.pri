INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

# SSL support is optional. Enable it with: qmake CONFIG+=ssl
contains(CONFIG, ssl) {
    DEFINES += HOBRASOFT_HTTPD_SSL
}

HEADERS += \
 $$PWD/hobrasofthttpd.h \
 $$PWD/httpserver.h \
 $$PWD/httpconnection.h \
 $$PWD/httpcookie.h \
 $$PWD/httprequest.h \
 $$PWD/httprequesthandler.h \
 $$PWD/httpresponse.h \
 $$PWD/shtmlcontroller.h \
 $$PWD/staticfilecontroller.h \
 $$PWD/httpsession.h \
 $$PWD/httpsessionstore.h \
 $$PWD/httpsettings.h \
 $$PWD/httptcpserver.h \
 $$PWD/httpgzipcompression.h \
 $$PWD/httpthread.h \
 $$PWD/testsettings.h \


SOURCES += \
 $$PWD/httpserver.cpp \
 $$PWD/httpconnection.cpp \
 $$PWD/httpcookie.cpp \
 $$PWD/httprequest.cpp \
 $$PWD/httprequesthandler.cpp \
 $$PWD/httpresponse.cpp \
 $$PWD/shtmlcontroller.cpp \
 $$PWD/staticfilecontroller.cpp \
 $$PWD/httpsession.cpp \
 $$PWD/httpsessionstore.cpp \
 $$PWD/httpsettings.cpp \
 $$PWD/httpgzipcompression.cpp \
 $$PWD/httptcpserver.cpp \
 $$PWD/httpthread.cpp \
