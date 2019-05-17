





#include "intro.h"
#include "ui_intro.h"

#include "guiutil.h"

#include "util.h"

#include <boost/filesystem.hpp>

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>


static const uint64_t GB_BYTES = 1000000000LL;
static const uint64_t BLOCK_CHAIN_SIZE = 1LL * GB_BYTES;

/* Check free space asynchronously to prevent hanging the UI thread.

   Up to one request to check a path is in flight to this thread; when the check()
   function runs, the current path is requested from the associated Intro object.
   The reply is sent back through a signal.

   This ensures that no queue of checking requests is built up while the user is
   still entering the path, and that always the most recently entered path is checked as
   soon as the thread becomes available.
*/
class FreespaceChecker : public QObject
{
    Q_OBJECT

public:
    FreespaceChecker(Intro* intro);

    enum Status {
        ST_OK,
        ST_ERROR
    };

public slots:
    void check();

signals:
    void reply(int status, const QString& message, quint64 available);

private:
    Intro* intro;
};

#include "intro.moc"

FreespaceChecker::FreespaceChecker(Intro* intro)
{
    this->intro = intro;
}

void FreespaceChecker::check()
{
    namespace fs = boost::filesystem;
    QString dataDirStr = intro->getPathToCheck();
    fs::path dataDir = GUIUtil::qstringToBoostPath(dataDirStr);
    uint64_t freeBytesAvailable = 0;
    int replyStatus = ST_OK;
    QString replyMessage = tr("A new data directory will be created.");

    
    fs::path parentDir = dataDir;
    fs::path parentDirOld = fs::path();
    while (parentDir.has_parent_path() && !fs::exists(parentDir)) {
        parentDir = parentDir.parent_path();

        
        if (parentDirOld == parentDir)
            break;

        parentDirOld = parentDir;
    }

    try {
        freeBytesAvailable = fs::space(parentDir).available;
        if (fs::exists(dataDir)) {
            if (fs::is_directory(dataDir)) {
                QString separator = "<code>" + QDir::toNativeSeparators("/") + tr("name") + "</code>";
                replyStatus = ST_OK;
                replyMessage = tr("Directory already exists. Add %1 if you intend to create a new directory here.").arg(separator);
            } else {
                replyStatus = ST_ERROR;
                replyMessage = tr("Path already exists, and is not a directory.");
            }
        }
    } catch (fs::filesystem_error& e) {
        
        replyStatus = ST_ERROR;
        replyMessage = tr("Cannot create data directory here.");
    }
    emit reply(replyStatus, replyMessage, freeBytesAvailable);
}


Intro::Intro(QWidget* parent) : QDialog(parent),
                                ui(new Ui::Intro),
                                thread(0),
                                signalled(false)
{
    ui->setupUi(this);
    ui->sizeWarningLabel->setText(ui->sizeWarningLabel->text().arg(BLOCK_CHAIN_SIZE / GB_BYTES));
    startThread();
}

Intro::~Intro()
{
    delete ui;
    
    emit stopThread();
    thread->wait();
}

QString Intro::getDataDirectory()
{
    return ui->dataDirectory->text();
}

void Intro::setDataDirectory(const QString& dataDir)
{
    ui->dataDirectory->setText(dataDir);
    if (dataDir == getDefaultDataDirectory()) {
        ui->dataDirDefault->setChecked(true);
        ui->dataDirectory->setEnabled(false);
        ui->ellipsisButton->setEnabled(false);
    } else {
        ui->dataDirCustom->setChecked(true);
        ui->dataDirectory->setEnabled(true);
        ui->ellipsisButton->setEnabled(true);
    }
}

QString Intro::getDefaultDataDirectory()
{
    return GUIUtil::boostPathToQString(GetDefaultDataDir());
}

bool Intro::pickDataDirectory()
{
    namespace fs = boost::filesystem;
    QSettings settings;
    /* If data directory provided on command line, no need to look at settings
       or show a picking dialog */
    if (!GetArg("-datadir", "").empty())
        return true;
    
    QString dataDir = getDefaultDataDirectory();
    
    dataDir = settings.value("strDataDir", dataDir).toString();

    if (!fs::exists(GUIUtil::qstringToBoostPath(dataDir)) || GetBoolArg("-choosedatadir", false)) {
        
        Intro intro;
        intro.setDataDirectory(dataDir);
        intro.setWindowIcon(QIcon(":icons/bitcoin"));

        while (true) {
            if (!intro.exec()) {
                
                return false;
            }
            dataDir = intro.getDataDirectory();
            try {
                TryCreateDirectory(GUIUtil::qstringToBoostPath(dataDir));
                break;
            } catch (fs::filesystem_error& e) {
                QMessageBox::critical(0, tr("TESRA Core"),
                    tr("Error: Specified data directory \"%1\" cannot be created.").arg(dataDir));
                
            }
        }

        settings.setValue("strDataDir", dataDir);
    }
    /* Only override -datadir if different from the default, to make it possible to
     * override -datadir in the tesra.conf file in the default data directory
     * (to be consistent with tesrad behavior)
     */
    if (dataDir != getDefaultDataDirectory())
        SoftSetArg("-datadir", GUIUtil::qstringToBoostPath(dataDir).string()); 
    return true;
}

void Intro::setStatus(int status, const QString& message, quint64 bytesAvailable)
{
    switch (status) {
    case FreespaceChecker::ST_OK:
        ui->errorMessage->setText(message);
        ui->errorMessage->setStyleSheet("");
        break;
    case FreespaceChecker::ST_ERROR:
        ui->errorMessage->setText(tr("Error") + ": " + message);
        ui->errorMessage->setStyleSheet("QLabel { color: #800000 }");
        break;
    }
    
    if (status == FreespaceChecker::ST_ERROR) {
        ui->freeSpace->setText("");
    } else {
        QString freeString = tr("%1 GB of free space available").arg(bytesAvailable / GB_BYTES);
        if (bytesAvailable < BLOCK_CHAIN_SIZE) {
            freeString += " " + tr("(of %1 GB needed)").arg(BLOCK_CHAIN_SIZE / GB_BYTES);
            ui->freeSpace->setStyleSheet("QLabel { color: #800000 }");
        } else {
            ui->freeSpace->setStyleSheet("");
        }
        ui->freeSpace->setText(freeString + ".");
    }
    
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(status != FreespaceChecker::ST_ERROR);
}

void Intro::on_dataDirectory_textChanged(const QString& dataDirStr)
{
    
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    checkPath(dataDirStr);
}

void Intro::on_ellipsisButton_clicked()
{
    QString dir = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(0, "Choose data directory", ui->dataDirectory->text()));
    if (!dir.isEmpty())
        ui->dataDirectory->setText(dir);
}

void Intro::on_dataDirDefault_clicked()
{
    setDataDirectory(getDefaultDataDirectory());
}

void Intro::on_dataDirCustom_clicked()
{
    ui->dataDirectory->setEnabled(true);
    ui->ellipsisButton->setEnabled(true);
}

void Intro::startThread()
{
    thread = new QThread(this);
    FreespaceChecker* executor = new FreespaceChecker(this);
    executor->moveToThread(thread);

    connect(executor, SIGNAL(reply(int, QString, quint64)), this, SLOT(setStatus(int, QString, quint64)));
    connect(this, SIGNAL(requestCheck()), executor, SLOT(check()));
    
    connect(this, SIGNAL(stopThread()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopThread()), thread, SLOT(quit()));

    thread->start();
}

void Intro::checkPath(const QString& dataDir)
{
    mutex.lock();
    pathToCheck = dataDir;
    if (!signalled) {
        signalled = true;
        emit requestCheck();
    }
    mutex.unlock();
}

QString Intro::getPathToCheck()
{
    QString retval;
    mutex.lock();
    retval = pathToCheck;
    signalled = false; 
    mutex.unlock();
    return retval;
}
