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
#include <QStringList>

namespace HobrasoftHttpd {

class HttpSessionStore;

/**
 * @brief Information about one session
 *
 * Please, do not use constructors. The session should be created calling HttpSessionStore functions only, otherwise 
 * the session data will be never deleted.
 *
 * It is complicated to make constructors private and there is no warranty that the private constructor will
 * work accross Qt versions.
 */
class HttpSession {
  public:

    /**
     * @brief Returns session ID
     */
    QByteArray id() const { 
        QMutex &mutex = const_cast<QMutex &>(m_mutex);
        QMutexLocker locker(&mutex);
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
    bool contains(const QString& key) const {
        QMutex &mutex = const_cast<QMutex &>(m_mutex);
        QMutexLocker locker(&mutex);
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
    QVariant value(const QString& key) const { 
        QMutex &mutex = const_cast<QMutex &>(m_mutex);
        QMutexLocker locker(&mutex);
        return (m_data == NULL) ? QVariant() : m_data->values.value(key); 
        }

    /**
     * @brief Return time of last access to the session
     */
    QDateTime lastAccess() { 
        QMutex &mutex = const_cast<QMutex &>(m_mutex);
        QMutexLocker locker(&mutex);
        return (m_data == NULL) ? QDateTime() : m_data->lastAccess; 
        }

    /**
     * @brief Sets time of last access to the session
     */
    void setLastAccess() { 
        QMutexLocker locker(&m_mutex);
        if (m_data != NULL) {
            m_data->lastAccess = QDateTime::currentDateTime();
            }
        }

    QStringList keys() const {
        QStringList list;
        if (m_data != NULL) {
            QHashIterator<QString, QVariant> iterator(m_data->values);
            while (iterator.hasNext()) {
                iterator.next();
                list << iterator.key();
                }
            }
        return list;
        }


    QVariantHash variantHash() const {
        if (m_data == NULL) { return QVariantHash(); }
        return m_data->values;
        }


    /**
     * @brief Calls delete to free data of session. Called after the session is removed from session store
     */
    void freeData() {
        delete m_data;
        }

  private:
    /**
     * @brief Data of the session
     */
    struct HttpSessionData {
        QByteArray  id;                     ///< Session ID
        QDateTime   lastAccess;             ///< Time of last access
        QHash<QString, QVariant> values;    ///< List of values
        };

    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    HttpSessionData *m_data;
    QMutex           m_mutex;
    #endif


  public:
    /**
     * @brief Constructor, should be private. Do not use in your code
     */
    HttpSession();

    /**
     * @brief Copy constructor, should be private. Do not use in your code
     */
    HttpSession(const HttpSession& other);

    /**
     * @brief Operator=(), should be private. Do not use in your code
     */
    HttpSession& operator= (const HttpSession& other);

};

}

#endif
