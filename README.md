# Installation

## Download
    http://www.hobrasoft.cz/en/httpserver/hobrasofthttp-latest.tar.bz2
or
    git clone http://dev.hobrasoft.cz/diffusion/HTTPD/hobrasofthttpd.git


## Documentation
http://www.hobrasoft.cz/en/httpserver/doc/

## Installation
    qmake
    make

## Example
Start the example:
    cd example/htdocs
    ../bin/example

Example should listen on port 8080. 
It is important to change current directory to htdocs else the program could not be able to
find its web content. Then start web browser and load url http://localhost:8080/.


# Using hobrasoft httpd in your own code
Do not modify the http server. Extend it instead.


## Static content
If you want to serve static content only, you need only to set default options for your server:

    QSettings q;
    q.setValue("http/address",  "::");
    q.setValue("http/port",     8080);
    q.setValue("http/root"      "/path/to/htdocs");
    HobrasoftHttpd::HttpSettings *settings = new HobrasoftHttpd::HttpSettings(&q, parent);
    HobrasoftHttpd::HttpSettings *server   = new HobrasoftHttpd::HttpServer  (settings, parent);


## Dynamic content
If you want to access your internal C++ structures, you have to extend the class HttpServer 
and reimplement the HttpServer::requestHandler()

    #include "httpserver.h"
    #include "httprequesthandler.h"
    #include "myclass.h"

    using namespace HobrasoftHttpd;

    class RequestMapper : public HttpRequestHandler {
        Q_OBJECT
      public:
        RequestMapper(HttpConnection *parent);

        /*
           The service method is called every time when some request arrived.
           It should create new controller for each request and 
           pass the control to its service() method.

           If you want to server json content, you can derive your controllers
           from AbstractController class (see below)
         */
        void service(HttpRequest *request, HttpResponse *response) {
            if (request->path().startsWith("/my-function")) {
                HttpRequestHandler controller = new ControllerExample(connection());
                controller->service(request, response);
                return;
                }
            // call default handler - static html pages, shtml pages, images, javascript, styles...
            HttpRequestHandler::service(request, response);
            }
    };

    class MyHttpd : public HttpServer {
        Q_OBJECT
      public:
        Httpd(QObject *parent) : QObject(parent) { }
        // The method returns pointer to new instance of your own request mapper
        // The request mapper maps requests like "/my-function" to call your own classes
        HttpRequestHandler *requestHandler(HttpConnection *connection) {
            return new RequestMapper(connection);
            }
    };


## Dynamic JSON content
If you want to server your requests in JSON format, you can use the AbstractController class from example.
Extend the AbstractController class to serve your requests:

    class ControllerExample : public AbstractController {
        Q_OBJECT
      protected:
        virtual void serviceList (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response) {
            QVariantList list;
            list << "Item 1" << "Item 2" << "Item 3";
            serviceOK(request, response, list);
            }

        virtual void serviceIdGet (HobrasoftHttpd::HttpRequest *request, HobrasoftHttpd::HttpResponse *response, const QString& id) {
            QVariantMap data;
            data["id"] = id;
            data["name"] = "Item 1";
            data["description"] = "This is the item description";
            serviceOK(request, response, data);
            }

        virtual bool exists (const QString& id) const {
            return true;
            }

    };


## Changes 1.0 -> 1.1
If you have your application based on 1.0 version, you should modify your code when upgrading to 1.1 version:
- method HttpResponse::write() can be called repeatedly. Data are appended to buffer.
- method HttpResponse::write() writes the data to socket when the 
  flush() method was called. Flush is non-blocking and the HttpReponse object is deleted automatically when
  all data are written to the socket.

