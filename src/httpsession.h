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
#include <QSharedPointer>

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
    bool contains(const QString& key) const {
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
    QVariant value(const QString& key) const { 
        QMutexLocker locker(&m_mutex);
        return (m_data == NULL) ? QVariant() : m_data->values.value(key); 
        }

    /**
     * @brief Return time of last access to the session
     */
    QDateTime lastAccess() const { 
        QMutexLocker locker(&m_mutex);
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
    QSharedPointer<HttpSessionData> m_data;
    mutable QMutex   m_mutex;
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
