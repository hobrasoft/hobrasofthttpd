/**
 * @file
 * 
 * @author Petr Bravenec <petr.bravenec@hobrasoft.cz>
 */
#ifndef _Test_H_
#define _Test_H_

#include <QObject>
#include <QtTest/QtTest>

/**
 * @brief
 */
class Test : public QObject {
    Q_OBJECT
  public:
    Test();

  private slots:
    void    initTestCase();

    void    settingsDefaultValues();
    void    settingsCustomValues1();
    void    httpserverStartIPv4();
    void    httpserverStartIPv6();
    void    httpserverHttpService();

};

#endif

