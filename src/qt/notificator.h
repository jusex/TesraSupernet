



#ifndef BITCOIN_QT_NOTIFICATOR_H
#define BITCOIN_QT_NOTIFICATOR_H

#if defined(HAVE_CONFIG_H)
#include "config/tesra-config.h"
#endif

#include <QIcon>
#include <QObject>

QT_BEGIN_NAMESPACE
class QSystemTrayIcon;

#ifdef USE_DBUS
class QDBusInterface;
#endif
QT_END_NAMESPACE


class Notificator : public QObject
{
    Q_OBJECT

public:
    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    Notificator(const QString& programName, QSystemTrayIcon* trayIcon, QWidget* parent);
    ~Notificator();

    
    enum Class {
        Information, 
        Warning,     
        Critical     
    };

public slots:
    /** Show notification message.
       @param[in] cls    general message class
       @param[in] title  title shown with message
       @param[in] text   message content
       @param[in] icon   optional icon to show with message
       @param[in] millisTimeout notification timeout in milliseconds (defaults to 10 seconds)
       @note Platform implementations are free to ignore any of the provided fields except for \a text.
     */
    void notify(Class cls, const QString& title, const QString& text, const QIcon& icon = QIcon(), int millisTimeout = 10000);

private:
    QWidget* parent;
    enum Mode {
        None,                  
        Freedesktop,           
        QSystemTray,           
        Growl12,               
        Growl13,               
        UserNotificationCenter 
    };
    QString programName;
    Mode mode;
    QSystemTrayIcon* trayIcon;
#ifdef USE_DBUS
    QDBusInterface* interface;

    void notifyDBus(Class cls, const QString& title, const QString& text, const QIcon& icon, int millisTimeout);
#endif
    void notifySystray(Class cls, const QString& title, const QString& text, const QIcon& icon, int millisTimeout);
#ifdef Q_OS_MAC
    void notifyGrowl(Class cls, const QString& title, const QString& text, const QIcon& icon);
    void notifyMacUserNotificationCenter(Class cls, const QString& title, const QString& text, const QIcon& icon);
#endif
};

#endif 
