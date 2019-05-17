



#ifndef BITCOIN_QT_NETWORKSTYLE_H
#define BITCOIN_QT_NETWORKSTYLE_H

#include <QIcon>
#include <QPixmap>
#include <QString>


class NetworkStyle
{
public:
    
    static const NetworkStyle* instantiate(const QString& networkId);

    const QString& getAppName() const { return appName; }
    const QIcon& getAppIcon() const { return appIcon; }
    const QString& getTitleAddText() const { return titleAddText; }
    const QPixmap& getSplashImage() const { return splashImage; }

private:
    NetworkStyle(const QString& appName, const QString& appIcon, const char* titleAddText, const QString& splashImage);

    QString appName;
    QIcon appIcon;
    QString titleAddText;
    QPixmap splashImage;
};

#endif 
