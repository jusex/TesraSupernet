



#ifndef BITCOIN_QT_OPTIONSMODEL_H
#define BITCOIN_QT_OPTIONSMODEL_H

#include "amount.h"

#include <QAbstractListModel>

QT_BEGIN_NAMESPACE
class QNetworkProxy;
QT_END_NAMESPACE

/** Interface from Qt to configuration data structure for Bitcoin client.
   To Qt, the options are presented as a list with the different options
   laid out vertically.
   This can be changed to a tree once the settings become sufficiently
   complex.
 */
class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit OptionsModel(QObject* parent = 0);

    enum OptionID {
        StartAtStartup,      
        MinimizeToTray,      
        MapPortUPnP,         
        MinimizeOnClose,     
        ProxyUse,            
        ProxyIP,             
        ProxyPort,           
        DisplayUnit,         
        ThirdPartyTxUrls,    
        Digits,              
        Theme,               
        Language,            
        CoinControlFeatures, 
        ThreadsScriptVerif,  
        DatabaseCache,       
        SpendZeroConfChange, 
        ZeromintPercentage,  
        ZeromintPrefDenom,   
        AnonymizeTesraAmount, 
        ShowMasternodesTab,  
        Listen,              
        OptionIDRowCount,
    };

    void Init();
    void Reset();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    
    void setDisplayUnit(const QVariant& value);

    
    bool getMinimizeToTray() { return fMinimizeToTray; }
    bool getMinimizeOnClose() { return fMinimizeOnClose; }
    int getDisplayUnit() { return nDisplayUnit; }
    QString getThirdPartyTxUrls() { return strThirdPartyTxUrls; }
    bool getProxySettings(QNetworkProxy& proxy) const;
    bool getCoinControlFeatures() { return fCoinControlFeatures; }
    const QString& getOverriddenByCommandLine() { return strOverriddenByCommandLine; }

    
    void setRestartRequired(bool fRequired);
    bool isRestartRequired();
    bool resetSettings;

private:
    
    bool fMinimizeToTray;
    bool fMinimizeOnClose;
    QString language;
    int nDisplayUnit;
    QString strThirdPartyTxUrls;
    bool fCoinControlFeatures;
    
    QString strOverriddenByCommandLine;

    
    void addOverriddenOption(const std::string& option);

signals:
    void displayUnitChanged(int unit);
    void zeromintPercentageChanged(int);
    void preferredDenomChanged(int);
    void anonymizeTesraAmountChanged(int);
    void coinControlFeaturesChanged(bool);
};

#endif 
