



#ifndef BITCOIN_QT_RPCCONSOLE_H
#define BITCOIN_QT_RPCCONSOLE_H

#include "guiutil.h"
#include "peertablemodel.h"

#include "net.h"

#include <QDialog>
#include <QCompleter>

class ClientModel;

namespace Ui
{
class RPCConsole;
}

QT_BEGIN_NAMESPACE
class QItemSelection;
QT_END_NAMESPACE


class RPCConsole : public QDialog
{
    Q_OBJECT

public:
    explicit RPCConsole(QWidget* parent);
    ~RPCConsole();

    static bool parseCommandLine(std::vector<std::string>& args, const std::string& strCommand);


    void setClientModel(ClientModel* model);

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

protected:
    virtual bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void on_lineEdit_returnPressed();
    void on_tabWidget_currentChanged(int index);
    
    void on_openDebugLogfileButton_clicked();
    
    void on_sldGraphRange_valueChanged(int value);
    
    void updateTrafficStats(quint64 totalBytesIn, quint64 totalBytesOut);
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);

public slots:
    void clear();

    
    void walletSalvage();
    void walletRescan();
    void walletZaptxes1();
    void walletZaptxes2();
    void walletUpgrade();
    void walletReindex();
    void walletResync();

    void reject();
    void message(int category, const QString& message, bool html = false);
    
    void setNumConnections(int count);
    
    void setNumBlocks(int count);
    
    void setMasternodeCount(const QString& strMasternodes);
    
    void browseHistory(int offset);
    
    void scrollToEnd();
    
    void showInfo();
    
    void showConsole();
    
    void showNetwork();
    
    void showPeers();
    
    void showRepair();
    
    void showConfEditor();
    
    void showMNConfEditor();
    
    void peerSelected(const QItemSelection& selected, const QItemSelection& deselected);
    
    void peerLayoutChanged();
    
    void showBackups();

signals:
    
    void stopExecutor();
    void cmdRequest(const QString& command);
    
    void handleRestart(QStringList args);

private:
    static QString FormatBytes(quint64 bytes);
    void startExecutor();
    void setTrafficGraphRange(int mins);
    
    void buildParameterlist(QString arg);
    
    void updateNodeDetail(const CNodeCombinedStats* stats);

    enum ColumnWidths {
        ADDRESS_COLUMN_WIDTH = 170,
        SUBVERSION_COLUMN_WIDTH = 140,
        PING_COLUMN_WIDTH = 80
    };

    Ui::RPCConsole* ui;
    ClientModel* clientModel;
    QStringList history;
    int historyPtr;
    NodeId cachedNodeid;
    QCompleter *autoCompleter;
};

#endif 
