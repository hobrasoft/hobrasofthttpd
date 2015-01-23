#ifndef _Item_H_
#define _Item_H_

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>

namespace Example {

/**
 * @brief One data item in the example
 *
 * The value of the item is incremented periodically and then the statusChanged() is signaled.
 */
class Item : public QObject {
    Q_OBJECT
  public:
    /**
     * @brief Constructor creates item from given parameters
     */
    Item(const QString& id, const QString& text, int interval, QObject *parent);

    const QString& id() const;      ///< Returns id of the item

    const QString& text() const;    ///< Returns text of the item

    QVariantMap webStatus() const;  ///< Returns webStatus of the item.

  signals:
    /**
     * @brief Signal is invoked when the status of the item changed
     */
    void    statusChanged(const Item *);

  private slots:
    /**
     * @brief Slot is called periodicaly to increment inner value of the example item
     */
    void    updateStatus();

  private:
    #ifndef DOXYGEN_SHOULD_SKIP_THIS
    QString m_id;
    QString m_text;
    int     m_counter;

    QTimer *m_timer;
    #endif
};

}

#endif
