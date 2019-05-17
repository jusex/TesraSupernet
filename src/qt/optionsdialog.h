



#ifndef BITCOIN_QT_OPTIONSDIALOG_H
#define BITCOIN_QT_OPTIONSDIALOG_H

#include <QDialog>

class OptionsModel;
class QValidatedLineEdit;

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui
{
class OptionsDialog;
}


class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget* parent, bool enableWallet);
    ~OptionsDialog();

    void setModel(OptionsModel* model);
    void setMapper();

protected:
    bool eventFilter(QObject* object, QEvent* event);

private slots:
    
    void enableOkButton();
    
    void disableOkButton();
    
    void setOkButtonState(bool fState);
    void on_resetButton_clicked();
    void on_okButton_clicked();
    void on_cancelButton_clicked();

    void showRestartWarning(bool fPersistent = false);
    void clearStatusLabel();
    void doProxyIpChecks(QValidatedLineEdit* pUiProxyIp, int nProxyPort);

signals:
    void proxyIpChecks(QValidatedLineEdit* pUiProxyIp, int nProxyPort);

private:
    Ui::OptionsDialog* ui;
    OptionsModel* model;
    QDataWidgetMapper* mapper;
    bool fProxyIpValid;
};

#endif 
