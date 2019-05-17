



#ifndef BITCOIN_QT_TEST_PAYMENTSERVERTESTS_H
#define BITCOIN_QT_TEST_PAYMENTSERVERTESTS_H

#include "../paymentserver.h"

#include <QObject>
#include <QTest>

class PaymentServerTests : public QObject
{
    Q_OBJECT

private slots:
    void paymentServerTests();
};




class RecipientCatcher : public QObject
{
    Q_OBJECT

public slots:
    void getRecipient(SendCoinsRecipient r);

public:
    SendCoinsRecipient recipient;
};

#endif 
