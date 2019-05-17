





#include "rpcconsole.h"
#include "ui_rpcconsole.h"

#include "clientmodel.h"
#include "guiutil.h"
#include "peertablemodel.h"

#include "chainparams.h"
#include "main.h"
#include "rpcclient.h"
#include "rpcserver.h"
#include "util.h"

#include <openssl/crypto.h>

#include <univalue.h>

#ifdef ENABLE_WALLET
#include <db_cxx.h>
#endif

#include <QDir>
#include <QKeyEvent>
#include <QScrollBar>
#include <QThread>
#include <QTime>
#include <QStringList>

#if QT_VERSION < 0x050000
#include <QUrl>
#endif





const int CONSOLE_HISTORY = 50;
const QSize ICON_SIZE(24, 24);

const int INITIAL_TRAFFIC_GRAPH_MINS = 30;


const QString SALVAGEWALLET("-salvagewallet");
const QString RESCAN("-rescan");
const QString ZAPTXES1("-zapwallettxes=1");
const QString ZAPTXES2("-zapwallettxes=2");
const QString UPGRADEWALLET("-upgradewallet");
const QString REINDEX("-reindex");
const QString RESYNC("-resync");

const struct {
    const char* url;
    const char* source;
} ICON_MAPPING[] = {
    {"cmd-request", ":/icons/tx_input"},
    {"cmd-reply", ":/icons/tx_output"},
    {"cmd-error", ":/icons/tx_output"},
    {"misc", ":/icons/tx_inout"},
    {NULL, NULL}};

/* Object for executing console RPC commands in a separate thread.
*/
class RPCExecutor : public QObject
{
    Q_OBJECT

public slots:
    void request(const QString& command);

signals:
    void reply(int category, const QString& command);
};

#include "rpcconsole.moc"






/**
 * Split shell command line into a list of arguments. Aims to emulate \c bash and friends.
 *
 * - Arguments are delimited with whitespace
 * - Extra whitespace at the beginning and end and between arguments will be ignored
 * - Text can be "double" or 'single' quoted
 * - The backslash \c \ is used as escape character
 *   - Outside quotes, any character can be escaped
 *   - Within double quotes, only escape \c " and backslashes before a \c " or another backslash
 *   - Within single quotes, no escaping is possible and no special interpretation takes place
 *
 * @param[out]   args        Parsed arguments will be appended to this list
 * @param[in]    strCommand  Command line to split
 */
bool RPCConsole::parseCommandLine(std::vector<std::string>& args, const std::string& strCommand)
{
    enum CmdParseState {
        STATE_EATING_SPACES,
        STATE_ARGUMENT,
        STATE_SINGLEQUOTED,
        STATE_DOUBLEQUOTED,
        STATE_ESCAPE_OUTER,
        STATE_ESCAPE_DOUBLEQUOTED
    } state = STATE_EATING_SPACES;
    std::string curarg;
    foreach (char ch, strCommand) {
        switch (state) {
        case STATE_ARGUMENT:      
        case STATE_EATING_SPACES: 
            switch (ch) {
            case '"':
                state = STATE_DOUBLEQUOTED;
                break;
            case '\'':
                state = STATE_SINGLEQUOTED;
                break;
            case '\\':
                state = STATE_ESCAPE_OUTER;
                break;
            case ' ':
            case '\n':
            case '\t':
                if (state == STATE_ARGUMENT) 
                {
                    args.push_back(curarg);
                    curarg.clear();
                }
                state = STATE_EATING_SPACES;
                break;
            default:
                curarg += ch;
                state = STATE_ARGUMENT;
            }
            break;
        case STATE_SINGLEQUOTED: 
            switch (ch) {
            case '\'':
                state = STATE_ARGUMENT;
                break;
            default:
                curarg += ch;
            }
            break;
        case STATE_DOUBLEQUOTED: 
            switch (ch) {
            case '"':
                state = STATE_ARGUMENT;
                break;
            case '\\':
                state = STATE_ESCAPE_DOUBLEQUOTED;
                break;
            default:
                curarg += ch;
            }
            break;
        case STATE_ESCAPE_OUTER: 
            curarg += ch;
            state = STATE_ARGUMENT;
            break;
        case STATE_ESCAPE_DOUBLEQUOTED:                  
            if (ch != '"' && ch != '\\') curarg += '\\'; 
            curarg += ch;
            state = STATE_DOUBLEQUOTED;
            break;
        }
    }
    switch (state) 
    {
    case STATE_EATING_SPACES:
        return true;
    case STATE_ARGUMENT:
        args.push_back(curarg);
        return true;
    default: 
        return false;
    }
}

void RPCExecutor::request(const QString& command)
{
    std::vector<std::string> args;
    if (!RPCConsole::parseCommandLine(args, command.toStdString())) {
        emit reply(RPCConsole::CMD_ERROR, QString("Parse error: unbalanced1 ' or \""));
        return;
    }
    if (args.empty())
        return; 
    try {
        std::string strPrint;
        
        
        UniValue result = tableRPC.execute(
            args[0],
            RPCConvertValues(args[0], std::vector<std::string>(args.begin() + 1, args.end())));

        
        if (result.isNull())
            strPrint = "";
        else if (result.isStr())
            strPrint = result.get_str();
        else
            strPrint = result.write(2);

        emit reply(RPCConsole::CMD_REPLY, QString::fromStdString(strPrint));
    } catch (UniValue& objError) {
        try 
        {
            int code = find_value(objError, "code").get_int();
            std::string message = find_value(objError, "message").get_str();
            emit reply(RPCConsole::CMD_ERROR, QString::fromStdString(message) + " (code " + QString::number(code) + ")");
        } catch (std::runtime_error&) 
        {                             
            emit reply(RPCConsole::CMD_ERROR, QString::fromStdString(objError.write()));
        }
    } catch (std::exception& e) {
        emit reply(RPCConsole::CMD_ERROR, QString("Error: ") + QString::fromStdString(e.what()));
    }
}

RPCConsole::RPCConsole(QWidget* parent) : QDialog(parent),
                                          ui(new Ui::RPCConsole),
                                          clientModel(0),
                                          historyPtr(0),
                                          cachedNodeid(-1)
{
    ui->setupUi(this);
    GUIUtil::restoreWindowGeometry("nRPCConsoleWindow", this->size(), this);

#ifndef Q_OS_MAC
    ui->openDebugLogfileButton->setIcon(QIcon(":/icons/export"));
#endif

    
    ui->lineEdit->installEventFilter(this);
    ui->messagesWidget->installEventFilter(this);

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(ui->btnClearTrafficGraph, SIGNAL(clicked()), ui->trafficGraph, SLOT(clear()));

    
    connect(ui->btn_salvagewallet, SIGNAL(clicked()), this, SLOT(walletSalvage()));
    connect(ui->btn_rescan, SIGNAL(clicked()), this, SLOT(walletRescan()));
    connect(ui->btn_zapwallettxes1, SIGNAL(clicked()), this, SLOT(walletZaptxes1()));
    connect(ui->btn_zapwallettxes2, SIGNAL(clicked()), this, SLOT(walletZaptxes2()));
    connect(ui->btn_upgradewallet, SIGNAL(clicked()), this, SLOT(walletUpgrade()));
    connect(ui->btn_reindex, SIGNAL(clicked()), this, SLOT(walletReindex()));
    connect(ui->btn_resync, SIGNAL(clicked()), this, SLOT(walletResync()));

    
    ui->openSSLVersion->setText(SSLeay_version(SSLEAY_VERSION));
#ifdef ENABLE_WALLET
    ui->berkeleyDBVersion->setText(DbEnv::version(0, 0, 0));
    ui->wallet_path->setText(QString::fromStdString(GetDataDir().string() + QDir::separator().toLatin1() + GetArg("-wallet", "wallet.dat")));
#else
    ui->label_berkeleyDBVersion->hide();
    ui->berkeleyDBVersion->hide();
#endif

    startExecutor();
    setTrafficGraphRange(INITIAL_TRAFFIC_GRAPH_MINS);

    ui->peerHeading->setText(tr("Select a peer to view detailed information."));

    clear();
}

RPCConsole::~RPCConsole()
{
    GUIUtil::saveWindowGeometry("nRPCConsoleWindow", this);
    emit stopExecutor();
    delete ui;
}

bool RPCConsole::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) 
    {
        QKeyEvent* keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch (key) {
        case Qt::Key_Up:
            if (obj == ui->lineEdit) {
                browseHistory(-1);
                return true;
            }
            break;
        case Qt::Key_Down:
            if (obj == ui->lineEdit) {
                browseHistory(1);
                return true;
            }
            break;
        case Qt::Key_PageUp: 
        case Qt::Key_PageDown:
            if (obj == ui->lineEdit) {
                QApplication::postEvent(ui->messagesWidget, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            
            if(obj == autoCompleter->popup()) {
                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        default:
            
            
            if (obj == ui->messagesWidget && ((!mod && !keyevt->text().isEmpty() && key != Qt::Key_Tab) ||
                                                 ((mod & Qt::ControlModifier) && key == Qt::Key_V) ||
                                                 ((mod & Qt::ShiftModifier) && key == Qt::Key_Insert))) {
                ui->lineEdit->setFocus();
                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

void RPCConsole::setClientModel(ClientModel* model)
{
    clientModel = model;
    ui->trafficGraph->setClientModel(model);
    if (model) {
        
        setNumConnections(model->getNumConnections());
        connect(model, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(model->getNumBlocks());
        connect(model, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

        setMasternodeCount(model->getMasternodeCountString());
        connect(model, SIGNAL(strMasternodesChanged(QString)), this, SLOT(setMasternodeCount(QString)));

        updateTrafficStats(model->getTotalBytesRecv(), model->getTotalBytesSent());
        connect(model, SIGNAL(bytesChanged(quint64, quint64)), this, SLOT(updateTrafficStats(quint64, quint64)));

        
        ui->peerWidget->setModel(model->getPeerTableModel());
        ui->peerWidget->verticalHeader()->hide();
        ui->peerWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->peerWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->peerWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->peerWidget->setColumnWidth(PeerTableModel::Address, ADDRESS_COLUMN_WIDTH);
        ui->peerWidget->setColumnWidth(PeerTableModel::Subversion, SUBVERSION_COLUMN_WIDTH);
        ui->peerWidget->setColumnWidth(PeerTableModel::Ping, PING_COLUMN_WIDTH);

        
        connect(ui->peerWidget->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(peerSelected(const QItemSelection&, const QItemSelection&)));
        connect(model->getPeerTableModel(), SIGNAL(layoutChanged()), this, SLOT(peerLayoutChanged()));

        
        ui->clientVersion->setText(model->formatFullVersion());
        ui->clientName->setText(model->clientName());
        ui->buildDate->setText(model->formatBuildDate());
        ui->startupTime->setText(model->formatClientStartupTime());
        ui->networkName->setText(QString::fromStdString(Params().NetworkIDString()));

        
        QStringList wordList;
        std::vector<std::string> commandList = tableRPC.listCommands();
        for (size_t i = 0; i < commandList.size(); ++i)
        {
            wordList << commandList[i].c_str();
        }

        autoCompleter = new QCompleter(wordList, this);
        ui->lineEdit->setCompleter(autoCompleter);

        
        autoCompleter->popup()->installEventFilter(this);
    }
}

static QString categoryClass(int category)
{
    switch (category) {
    case RPCConsole::CMD_REQUEST:
        return "cmd-request";
        break;
    case RPCConsole::CMD_REPLY:
        return "cmd-reply";
        break;
    case RPCConsole::CMD_ERROR:
        return "cmd-error";
        break;
    default:
        return "misc";
    }
}


void RPCConsole::walletSalvage()
{
    buildParameterlist(SALVAGEWALLET);
}


void RPCConsole::walletRescan()
{
    buildParameterlist(RESCAN);
}


void RPCConsole::walletZaptxes1()
{
    buildParameterlist(ZAPTXES1);
}


void RPCConsole::walletZaptxes2()
{
    buildParameterlist(ZAPTXES2);
}


void RPCConsole::walletUpgrade()
{
    buildParameterlist(UPGRADEWALLET);
}


void RPCConsole::walletReindex()
{
    buildParameterlist(REINDEX);
}


void RPCConsole::walletResync()
{
    QString resyncWarning = tr("This will delete your local blockchain folders and the wallet will synchronize the complete Blockchain from scratch.<br /><br />");
        resyncWarning +=   tr("This needs quite some time and downloads a lot of data.<br /><br />");
        resyncWarning +=   tr("Your transactions and funds will be visible again after the download has completed.<br /><br />");
        resyncWarning +=   tr("Do you want to continue?.<br />");
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm resync Blockchain"),
        resyncWarning,
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (retval != QMessageBox::Yes) {
        
        return;
    }

    
    buildParameterlist(RESYNC);
}


void RPCConsole::buildParameterlist(QString arg)
{
    
    QStringList args = QApplication::arguments();
    args.removeFirst();

    
    args.removeAll(SALVAGEWALLET);
    args.removeAll(RESCAN);
    args.removeAll(ZAPTXES1);
    args.removeAll(ZAPTXES2);
    args.removeAll(UPGRADEWALLET);
    args.removeAll(REINDEX);

    
    args.append(arg);

    
    emit handleRestart(args);
}

void RPCConsole::clear()
{
    ui->messagesWidget->clear();
    history.clear();
    historyPtr = 0;
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();

    
    
    for (int i = 0; ICON_MAPPING[i].url; ++i) {
        ui->messagesWidget->document()->addResource(
            QTextDocument::ImageResource,
            QUrl(ICON_MAPPING[i].url),
            QImage(ICON_MAPPING[i].source).scaled(ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    
    ui->messagesWidget->document()->setDefaultStyleSheet(
        "table { }"
        "td.time { color: #808080; padding-top: 3px; } "
        "td.message { font-family: Courier, Courier New, Lucida Console, monospace; font-size: 12px; } " 
        "td.cmd-request { color: #006060; } "
        "td.cmd-error { color: red; } "
        "b { color: #006060; } ");

    message(CMD_REPLY, (tr("Welcome to the TESRA RPC console.") + "<br>" +
                           tr("Use up and down arrows to navigate history, and <b>Ctrl-L</b> to clear screen.") + "<br>" +
                           tr("Type <b>help</b> for an overview of available commands.")),
        true);
}

void RPCConsole::reject()
{
    
    if (windowType() != Qt::Widget)
        QDialog::reject();
}

void RPCConsole::message(int category, const QString& message, bool html)
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    QString out;
    out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
    out += "<td class=\"icon\" width=\"32\"><img src=\"" + categoryClass(category) + "\"></td>";
    out += "<td class=\"message " + categoryClass(category) + "\" valign=\"middle\">";
    if (html)
        out += message;
    else
        out += GUIUtil::HtmlEscape(message, true);
    out += "</td></tr></table>";
    ui->messagesWidget->append(out);
}

void RPCConsole::setNumConnections(int count)
{
    if (!clientModel)
        return;

    QString connections = QString::number(count) + " (";
    connections += tr("In:") + " " + QString::number(clientModel->getNumConnections(CONNECTIONS_IN)) + " / ";
    connections += tr("Out:") + " " + QString::number(clientModel->getNumConnections(CONNECTIONS_OUT)) + ")";

    ui->numberOfConnections->setText(connections);
}

void RPCConsole::setNumBlocks(int count)
{
    ui->numberOfBlocks->setText(QString::number(count));
    if (clientModel)
        ui->lastBlockTime->setText(clientModel->getLastBlockDate().toString());
}

void RPCConsole::setMasternodeCount(const QString& strMasternodes)
{
    ui->masternodeCount->setText(strMasternodes);
}

void RPCConsole::on_lineEdit_returnPressed()
{
    QString cmd = ui->lineEdit->text();
    ui->lineEdit->clear();

    if (!cmd.isEmpty()) {
        message(CMD_REQUEST, cmd);
        emit cmdRequest(cmd);
        
        history.removeOne(cmd);
        
        history.append(cmd);
        
        while (history.size() > CONSOLE_HISTORY)
            history.removeFirst();
        
        historyPtr = history.size();
        
        scrollToEnd();
    }
}

void RPCConsole::browseHistory(int offset)
{
    historyPtr += offset;
    if (historyPtr < 0)
        historyPtr = 0;
    if (historyPtr > history.size())
        historyPtr = history.size();
    QString cmd;
    if (historyPtr < history.size())
        cmd = history.at(historyPtr);
    ui->lineEdit->setText(cmd);
}

void RPCConsole::startExecutor()
{
    QThread* thread = new QThread;
    RPCExecutor* executor = new RPCExecutor();
    executor->moveToThread(thread);

    
    connect(executor, SIGNAL(reply(int, QString)), this, SLOT(message(int, QString)));
    
    connect(this, SIGNAL(cmdRequest(QString)), executor, SLOT(request(QString)));

    
    
    
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    
    
    thread->start();
}

void RPCConsole::on_tabWidget_currentChanged(int index)
{
    if (ui->tabWidget->widget(index) == ui->tab_console) {
        ui->lineEdit->setFocus();
    }
}

void RPCConsole::on_openDebugLogfileButton_clicked()
{
    GUIUtil::openDebugLogfile();
}

void RPCConsole::scrollToEnd()
{
    QScrollBar* scrollbar = ui->messagesWidget->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void RPCConsole::on_sldGraphRange_valueChanged(int value)
{
    const int multiplier = 5; 
    int mins = value * multiplier;
    setTrafficGraphRange(mins);
}

QString RPCConsole::FormatBytes(quint64 bytes)
{
    if (bytes < 1024)
        return QString(tr("%1 B")).arg(bytes);
    if (bytes < 1024 * 1024)
        return QString(tr("%1 KB")).arg(bytes / 1024);
    if (bytes < 1024 * 1024 * 1024)
        return QString(tr("%1 MB")).arg(bytes / 1024 / 1024);

    return QString(tr("%1 GB")).arg(bytes / 1024 / 1024 / 1024);
}

void RPCConsole::setTrafficGraphRange(int mins)
{
    ui->trafficGraph->setGraphRangeMins(mins);
    ui->lblGraphRange->setText(GUIUtil::formatDurationStr(mins * 60));
}

void RPCConsole::updateTrafficStats(quint64 totalBytesIn, quint64 totalBytesOut)
{
    ui->lblBytesIn->setText(FormatBytes(totalBytesIn));
    ui->lblBytesOut->setText(FormatBytes(totalBytesOut));
}

void RPCConsole::showInfo()
{
    ui->tabWidget->setCurrentIndex(0);
    show();
}

void RPCConsole::showConsole()
{
    ui->tabWidget->setCurrentIndex(1);
    show();
}

void RPCConsole::showNetwork()
{
    ui->tabWidget->setCurrentIndex(2);
    show();
}

void RPCConsole::showPeers()
{
    ui->tabWidget->setCurrentIndex(3);
    show();
}

void RPCConsole::showRepair()
{
    ui->tabWidget->setCurrentIndex(4);
    show();
}

void RPCConsole::showConfEditor()
{
    GUIUtil::openConfigfile();
}

void RPCConsole::showMNConfEditor()
{
    GUIUtil::openMNConfigfile();
}

void RPCConsole::peerSelected(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected);

    if (!clientModel || selected.indexes().isEmpty())
        return;

    const CNodeCombinedStats* stats = clientModel->getPeerTableModel()->getNodeStats(selected.indexes().first().row());
    if (stats)
        updateNodeDetail(stats);
}

void RPCConsole::peerLayoutChanged()
{
    if (!clientModel)
        return;

    const CNodeCombinedStats* stats = NULL;
    bool fUnselect = false;
    bool fReselect = false;

    if (cachedNodeid == -1) 
        return;

    
    int selectedRow;
    QModelIndexList selectedModelIndex = ui->peerWidget->selectionModel()->selectedIndexes();
    if (selectedModelIndex.isEmpty())
        selectedRow = -1;
    else
        selectedRow = selectedModelIndex.first().row();

    
    
    int detailNodeRow = clientModel->getPeerTableModel()->getRowByNodeId(cachedNodeid);

    if (detailNodeRow < 0) {
        
        fUnselect = true;
        cachedNodeid = -1;
        ui->peerHeading->setText(tr("Select a peer to view detailed information."));
    } else {
        if (detailNodeRow != selectedRow) {
            
            fUnselect = true;
            fReselect = true;
        }

        
        stats = clientModel->getPeerTableModel()->getNodeStats(detailNodeRow);
    }

    if (fUnselect && selectedRow >= 0) {
        ui->peerWidget->selectionModel()->select(QItemSelection(selectedModelIndex.first(), selectedModelIndex.last()),
            QItemSelectionModel::Deselect);
    }

    if (fReselect) {
        ui->peerWidget->selectRow(detailNodeRow);
    }

    if (stats)
        updateNodeDetail(stats);
}

void RPCConsole::updateNodeDetail(const CNodeCombinedStats* stats)
{
    
    cachedNodeid = stats->nodeStats.nodeid;

    
    QString peerAddrDetails(QString::fromStdString(stats->nodeStats.addrName));
    if (!stats->nodeStats.addrLocal.empty())
        peerAddrDetails += "<br />" + tr("via %1").arg(QString::fromStdString(stats->nodeStats.addrLocal));
    ui->peerHeading->setText(peerAddrDetails);
    ui->peerServices->setText(GUIUtil::formatServicesStr(stats->nodeStats.nServices));
    ui->peerLastSend->setText(stats->nodeStats.nLastSend ? GUIUtil::formatDurationStr(GetTime() - stats->nodeStats.nLastSend) : tr("never"));
    ui->peerLastRecv->setText(stats->nodeStats.nLastRecv ? GUIUtil::formatDurationStr(GetTime() - stats->nodeStats.nLastRecv) : tr("never"));
    ui->peerBytesSent->setText(FormatBytes(stats->nodeStats.nSendBytes));
    ui->peerBytesRecv->setText(FormatBytes(stats->nodeStats.nRecvBytes));
    ui->peerConnTime->setText(GUIUtil::formatDurationStr(GetTime() - stats->nodeStats.nTimeConnected));
    ui->peerPingTime->setText(GUIUtil::formatPingTime(stats->nodeStats.dPingTime));
    ui->peerVersion->setText(QString("%1").arg(stats->nodeStats.nVersion));
    ui->peerSubversion->setText(QString::fromStdString(stats->nodeStats.cleanSubVer));
    ui->peerDirection->setText(stats->nodeStats.fInbound ? tr("Inbound") : tr("Outbound"));
    ui->peerHeight->setText(QString("%1").arg(stats->nodeStats.nStartingHeight));

    
    
    if (stats->fNodeStateStatsAvailable) {
        
        ui->peerBanScore->setText(QString("%1").arg(stats->nodeStateStats.nMisbehavior));

        
        if (stats->nodeStateStats.nSyncHeight > -1)
            ui->peerSyncHeight->setText(QString("%1").arg(stats->nodeStateStats.nSyncHeight));
        else
            ui->peerSyncHeight->setText(tr("Unknown"));
    } else {
        ui->peerBanScore->setText(tr("Fetching..."));
        ui->peerSyncHeight->setText(tr("Fetching..."));
    }

    ui->detailWidget->show();
}

void RPCConsole::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void RPCConsole::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (!clientModel)
        return;

    
    clientModel->getPeerTableModel()->startAutoRefresh();
}

void RPCConsole::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);

    if (!clientModel)
        return;

    
    clientModel->getPeerTableModel()->stopAutoRefresh();
}

void RPCConsole::showBackups()
{
    GUIUtil::showBackups();
}
