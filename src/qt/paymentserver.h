



#ifndef BITCOIN_QT_PAYMENTSERVER_H
#define BITCOIN_QT_PAYMENTSERVER_H




























#include "paymentrequestplus.h"
#include "walletmodel.h"

#include <QObject>
#include <QString>

class OptionsModel;

class CWallet;

QT_BEGIN_NAMESPACE
class QApplication;
class QByteArray;
class QLocalServer;
class QNetworkAccessManager;
class QNetworkReply;
class QSslError;
class QUrl;
QT_END_NAMESPACE


extern const qint64 BIP70_MAX_PAYMENTREQUEST_SIZE;

class PaymentServer : public QObject
{
    Q_OBJECT

public:
    
    
    static void ipcParseCommandLine(int argc, char* argv[]);

    
    
    
    
    
    static bool ipcSendCommandLine();

    
    PaymentServer(QObject* parent, bool startLocalServer = true);
    ~PaymentServer();

    
    
    
    
    
    static void LoadRootCAs(X509_STORE* store = NULL);

    
    static X509_STORE* getCertStore();

    
    void setOptionsModel(OptionsModel* optionsModel);

    
    static bool readPaymentRequestFromFile(const QString& filename, PaymentRequestPlus& request);

signals:
    
    void receivedPaymentRequest(SendCoinsRecipient);

    
    void receivedPaymentACK(const QString& paymentACKMsg);

    
    void message(const QString& title, const QString& message, unsigned int style);

public slots:
    
    
    void uiReady();

    
    void fetchPaymentACK(CWallet* wallet, SendCoinsRecipient recipient, QByteArray transaction);

    
    void handleURIOrFile(const QString& s);

private slots:
    void handleURIConnection();
    void netRequestFinished(QNetworkReply*);
    void reportSslErrors(QNetworkReply*, const QList<QSslError>&);
    void handlePaymentACK(const QString& paymentACKMsg);

protected:
    
    
    bool eventFilter(QObject* object, QEvent* event);

private:
    bool processPaymentRequest(PaymentRequestPlus& request, SendCoinsRecipient& recipient);
    void fetchRequest(const QUrl& url);

    
    void initNetManager();

    bool saveURIs; 
    QLocalServer* uriServer;
    QNetworkAccessManager* netManager;  
    OptionsModel* optionsModel;
};

#endif 
