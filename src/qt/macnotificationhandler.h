



#ifndef BITCOIN_QT_MACNOTIFICATIONHANDLER_H
#define BITCOIN_QT_MACNOTIFICATIONHANDLER_H

#include <QObject>

/** Macintosh-specific notification handler (supports UserNotificationCenter and Growl).
 */
class MacNotificationHandler : public QObject
{
    Q_OBJECT

public:
    /** shows a 10.8+ UserNotification in the UserNotificationCenter
     */
    void showNotification(const QString& title, const QString& text);

    
    void sendAppleScript(const QString& script);

    
    bool hasUserNotificationCenterSupport(void);
    static MacNotificationHandler* instance();
};


#endif 
