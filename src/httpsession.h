/**
 * @file
 *
 * @author Stefan Frings
 * @author Petr Bravenec petr.bravenec@hobrasoft.cz
 */

#ifndef _HttpSession_H_
#define _HttpSession_H_

#include <QDateTime>
#include <QByteArray>
#include <QVariant>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>

namespace HobrasoftHttpd {

/**
 * @brief Information about one session
 */
class HttpSession {
  public:

    /**
     * @brief Destruktor
     */
    virtual ~HttpSession();

    /**
     * @brief Construktor
     */
    HttpSession();

    /**
     * @brief Copy constructor
     */
    HttpSession(const HttpSession& other);

    /**
     * @brief Operator=
     */
    HttpSession& operator= (const HttpSession& other);

    /**
     * @brief Returns session ID
     */
    QByteArray id() { 
        QMutexLocker locker(&m_mutex);
        return (m_data == NULL) ? QByteArray() : m_data->id; 
        }

    /**
     * @brief Returns true if the session is valid (not null)
     */
    bool isNull() const { 
        return m_data != NULL; 
        }


    /**
     * @brief Returns true if the session contains key
     */
    bool contains(const QString& key) {
        QMutexLocker locker(&m_mutex);
        if (m_data != NULL) {
            return m_data->values.contains(key);
            }
        return false;
        }

    /**
     * @brief Adds an item to the session
     */
    void add(const QString& key, const QVariant& value) { 
        QMutexLocker locker(&m_mutex);
        if (m_data != NULL) {
            m_data->values[key] = value; 
            }
        }

    /**
     * @brief Removes an item from the session
     */
    void remove(const QString& key) { 
        QMutexLocker locker(&m_mutex);
        if (m_data != NULL) { 
            m_data->values.remove(key); 
            } 
        }

    /**
     * @brief Returns an item from the session
     */
    QVariant value(const QString& key) { 
        QMutexLocker locker(&m_mutex);
        return (m_data == NULL) ? QVariant() : m_data->values.value(key); 
        }

    /**
     * @brief Return time of last access to the session
     */
    qint64 lastAccess() { 
        QMutexLocker locker(&m_mutex);
        return (m_data == NULL) ? 0 : m_data->lastAccess; 
        }

    /**
     * @brief Sets time of last access to the session
     */
    void setLastAccess() { 
        QMutexLocker locker(&m_mutex);
        if (m_data != NULL) {
            m_data->lastAccess = QDateTime::currentDateTime().toTime_t()*1000; 
            }
        }

  private:
    /**
     * @brief Data of the session
     */
    struct HttpSessionData {
        QByteArray  id;                     ///< Session ID
        qint64      lastAccess;             ///< Time of last access
        int         refCount;               ///< Reference use counter
        QHash<QString, QVariant> values;    ///< List of values
        };

    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    HttpSessionData *m_data;
    QMutex           m_mutex;
    #endif
};

}

#endif
