/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _ShtmlController_H_
#define _ShtmlController_H_

#include <QObject>
#include <QCache>
#include "httprequesthandler.h"

namespace HobrasoftHttpd {

class HttpRequest;
class HttpResponse;
class HttpConnection;

/**
 * @brief Processes request to SHTML files
 * 
 * SHTML files can simply include other files.
 * You can use it to simply include headers and footer to your static files.
 *
 * The controller search for lines:
 *
 @code
 <!-- #include "soubor.html" -->
 @endcode
 * and replaces them with the content of included file.
 * There can be only one include pragma on the line. No other content is not allowed on the line.
 */
class ShtmlController : public HttpRequestHandler {
    Q_OBJECT
  public:

    /**
     * @brief Construct sets the default parameter from configuration (encoding and root)
     */
    ShtmlController(HttpConnection *parent);

    /**
     * @brief Processes one request
     */
    void service(HttpRequest *request, HttpResponse *response);

  private:
    /**
     * @brief Reads and processes one file, other files can be included recursivelly
     */
    QByteArray readFile(const QString& path, HttpResponse *response, int depth);

    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    HttpConnection  *m_parent;
    #endif

};

}

#endif
