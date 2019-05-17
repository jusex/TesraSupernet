



#ifndef BITCOIN_QT_PLATFORMSTYLE_H
#define BITCOIN_QT_PLATFORMSTYLE_H

#include <QIcon>
#include <QPixmap>
#include <QString>


class PlatformStyle
{
public:
    
    static const PlatformStyle *instantiate(const QString &platformId);

    const QString &getName() const { return name; }

    bool getImagesOnButtons() const { return imagesOnButtons; }
    bool getUseExtraSpacing() const { return useExtraSpacing; }

    QColor TextColor() const { return textColor; }
    QColor SingleColor() const { return singleColor; }
    QColor MenuColor() const { return menuColor; }

    
    QImage SingleColorImage(const QString& filename) const;

    
    QIcon SingleColorIcon(const QString& filename) const;

    
    QIcon SingleColorIcon(const QIcon& icon) const;

    
    QIcon TextColorIcon(const QString& filename) const;

    
    QIcon TextColorIcon(const QIcon& icon) const;

    
    QIcon MenuColorIcon(const QString& filename) const;

    
    QIcon MenuColorIcon(const QIcon& icon) const;

    enum StateType{
        NavBar = 0,
        PushButton = 1
    };
    
    QIcon MultiStatesIcon(const QString& resourcename, StateType type = NavBar, QColor color = Qt::white, QColor colorAlt = 0x2d2d2d) const;

    enum TableColorType{
        Normal = 0,
        Input,
        Inout,
        Output,
        Error
    };
    QIcon TableColorIcon(const QString& resourcename, TableColorType type) const;
    QImage TableColorImage(const QString& resourcename, TableColorType type) const;
    void TableColor(TableColorType type, int& color, double& opacity) const;
private:
    PlatformStyle(const QString &name, bool imagesOnButtons, bool colorizeIcons, bool useExtraSpacing);

    QString name;
    bool imagesOnButtons;
    bool colorizeIcons;
    bool useExtraSpacing;
    QColor singleColor;
    QColor textColor;
    QColor menuColor;
    
};

#endif 

