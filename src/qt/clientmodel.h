





#ifndef BITCOIN_QT_CLIENTMODEL_H
#define BITCOIN_QT_CLIENTMODEL_H

#include <QObject>

class AddressTableModel;
class OptionsModel;
class PeerTableModel;
class TransactionTableModel;

class CWallet;

QT_BEGIN_NAMESPACE
class QDateTime;
class QTimer;
QT_END_NAMESPACE

enum BlockSource {
    BLOCK_SOURCE_NONE,
    BLOCK_SOURCE_REINDEX,
    BLOCK_SOURCE_DISK,
    BLOCK_SOURCE_NETWORK
};

enum NumConnections {
    CONNECTIONS_NONE = 0,
    CONNECTIONS_IN = (1U << 0),
    CONNECTIONS_OUT = (1U << 1),
    CONNECTIONS_ALL = (CONNECTIONS_IN | CONNECTIONS_OUT),
};


class ClientModel : public QObject
{
    Q_OBJECT

public:
    explicit ClientModel(OptionsModel* optionsModel, QObject* parent = 0);
    ~ClientModel();

    OptionsModel* getOptionsModel();
    PeerTableModel* getPeerTableModel();

    
    int getNumConnections(unsigned int flags = CONNECTIONS_ALL) const;
    QString getMasternodeCountString() const;
    int getNumBlocks() const;
    int getNumBlocksAtStartup();

    quint64 getTotalBytesRecv() const;
    quint64 getTotalBytesSent() const;

    double getVerificationProgress() const;
    QDateTime getLastBlockDate() const;

    
    bool inInitialBlockDownload() const;
    
    enum BlockSource getBlockSource() const;
    
    QString getStatusBarWarnings() const;

    QString formatFullVersion() const;
    QString formatBuildDate() const;
    bool isReleaseVersion() const;
    QString clientName() const;
    QString formatClientStartupTime() const;

private:
    OptionsModel* optionsModel;
    PeerTableModel* peerTableModel;

    int cachedNumBlocks;
    QString cachedMasternodeCountString;
    bool cachedReindexing;
    bool cachedImporting;

    int numBlocksAtStartup;

    QTimer* pollTimer;
    QTimer* pollMnTimer;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();

signals:
    void numConnectionsChanged(int count);
    void numBlocksChanged(int count);
    void strMasternodesChanged(const QString& strMasternodes);
    void alertsChanged(const QString& warnings);
    void bytesChanged(quint64 totalBytesIn, quint64 totalBytesOut);

    
    void message(const QString& title, const QString& message, unsigned int style);

    
    void showProgress(const QString& title, int nProgress);

public slots:
    void updateTimer();
    void updateMnTimer();
    void updateNumConnections(int numConnections);
    void updateAlert(const QString& hash, int status);
};

#endif 
