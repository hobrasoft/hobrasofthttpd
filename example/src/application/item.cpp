/**
 * @file
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#include "item.h"

#include <QObject>
#include <QString>
#include <QVariantMap>

using namespace Example;

Item::Item(const QString& id, const QString& text, int interval, QObject *parent) : QObject(parent) {
    m_id = id;
    m_text = text;
    m_counter = 0;

    m_timer = new QTimer(this);
    m_timer->setInterval(interval);
    m_timer->setSingleShot(false);
    m_timer->start();
    connect (m_timer, SIGNAL(timeout()), this, SLOT(updateStatus()));
}


void Item::updateStatus() {
    m_counter++;
    emit statusChanged(this);
}


const QString& Item::id() const {
    return m_id;
}


const QString& Item::text() const {
    return m_text;
}


QVariantMap Item::webStatus() const {
    QVariantMap data;
    data["id"]      = m_id;
    data["text"]    = m_text;
    data["counter"] = m_counter;
    return data;
}

