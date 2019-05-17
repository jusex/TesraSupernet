



#ifndef BITCOIN_QT_SPLASHSCREEN_H
#define BITCOIN_QT_SPLASHSCREEN_H

#include <QSplashScreen>

class NetworkStyle;

/** Class for the splashscreen with information of the running client.
 *
 * @note this is intentionally not a QSplashScreen. Bitcoin Core initialization
 * can take a long time, and in that case a progress window that cannot be
 * moved around and minimized has turned out to be frustrating to the user.
 */
class SplashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SplashScreen(Qt::WindowFlags f, const NetworkStyle* networkStyle);
    ~SplashScreen();

protected:
    void paintEvent(QPaintEvent* event);
    void closeEvent(QCloseEvent* event);

public slots:
    
    void slotFinish(QWidget* mainWin);

    
    void showMessage(const QString& message, int alignment, const QColor& color);

private:
    
    void subscribeToCoreSignals();
    
    void unsubscribeFromCoreSignals();

    QPixmap pixmap;
    QString curMessage;
    QColor curColor;
    int curAlignment;
};

#endif 
