



#ifndef BITCOIN_QT_PAYMENTREQUESTPLUS_H
#define BITCOIN_QT_PAYMENTREQUESTPLUS_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "paymentrequest.pb.h"
#pragma GCC diagnostic pop

#include "base58.h"

#include <QByteArray>
#include <QList>
#include <QString>






class PaymentRequestPlus
{
public:
    PaymentRequestPlus() {}

    bool parse(const QByteArray& data);
    bool SerializeToString(std::string* output) const;

    bool IsInitialized() const;
    QString getPKIType() const;
    
    
    bool getMerchant(X509_STORE* certStore, QString& merchant) const;

    
    QList<std::pair<CScript, CAmount> > getPayTo() const;

    const payments::PaymentDetails& getDetails() const { return details; }

private:
    payments::PaymentRequest paymentRequest;
    payments::PaymentDetails details;
};

#endif 
