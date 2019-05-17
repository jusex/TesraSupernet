



#ifndef BITCOIN_QT_ADDRESSBOOKPAGE_H
#define BITCOIN_QT_ADDRESSBOOKPAGE_H

#include <QDialog>

class AddressTableModel;
class OptionsModel;

namespace Ui
{
class AddressBookPage;
}

QT_BEGIN_NAMESPACE
class QItemSelection;
class QMenu;
class QModelIndex;
class QSortFilterProxyModel;
class QTableView;
QT_END_NAMESPACE

/** Widget that shows a list of sending or receiving addresses.
  */
class AddressBookPage : public QDialog
{
    Q_OBJECT

public:
    enum Tabs {
        SendingTab = 0,
        ReceivingTab = 1
    };

    enum Mode {
        ForSelection, 
        ForEditing    
    };

    explicit AddressBookPage(Mode mode, Tabs tab, QWidget* parent);
    ~AddressBookPage();

    void setModel(AddressTableModel* model);
    const QString& getReturnValue() const { return returnValue; }

public slots:
    void done(int retval);

private:
    Ui::AddressBookPage* ui;
    AddressTableModel* model;
    Mode mode;
    Tabs tab;
    QString returnValue;
    QSortFilterProxyModel* proxyModel;
    QMenu* contextMenu;
    QAction* deleteAction; 
    QString newAddressToSelect;

private slots:
    
    void on_deleteAddress_clicked();
    
    void on_newAddress_clicked();
    
    void on_copyAddress_clicked();
    
    void onCopyLabelAction();
    
    void onEditAction();
    
    void on_exportButton_clicked();

    
    void selectionChanged();
    
    void contextualMenu(const QPoint& point);
    
    void selectNewAddress(const QModelIndex& parent, int begin, int );

signals:
    void sendCoins(QString addr);
};

#endif 
