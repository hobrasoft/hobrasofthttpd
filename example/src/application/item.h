#ifndef _Item_H_
#define _Item_H_

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>

class Item : public QObject {
    Q_OBJECT
  public:
    Item(const QString& id, const QString& text, int interval, QObject *parent);

    const QString& id() const;

    const QString& text() const;

    QVariantMap webStatus() const;

  signals:
    void    statusChanged(const Item *);

  private slots:
    void    updateStatus();

  private:
    QString m_id;
    QString m_text;
    int     m_counter;

    QTimer *m_timer;
};


#endif
