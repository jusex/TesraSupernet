





#ifndef BITCOIN_QT_GUICONSTANTS_H
#define BITCOIN_QT_GUICONSTANTS_H


static const int MODEL_UPDATE_DELAY = 250;


static const int MAX_PASSPHRASE_SIZE = 1024;


static const int STATUSBAR_ICONSIZE = 16;

static const bool DEFAULT_SPLASHSCREEN = true;


#define STYLE_INVALID "background:#FF8080"


#define COLOR_UNCONFIRMED QColor(128, 128, 128)

#define COLOR_NEGATIVE QColor(255, 0, 0)

#define COLOR_BAREADDRESS QColor(140, 140, 140)

#define COLOR_TX_STATUS_OPENUNTILDATE QColor(64, 64, 255)

#define COLOR_TX_STATUS_OFFLINE QColor(192, 192, 192)

#define COLOR_BLACK QColor(51, 51, 51)

/* Tooltips longer than this (in characters) are converted into rich text,
   so that they can be word-wrapped.
 */
static const int TOOLTIP_WRAP_THRESHOLD = 80;


static const int MAX_URI_LENGTH = 255;


#define EXPORT_IMAGE_SIZE 256


#define SPINNER_FRAMES 35

#define QAPP_ORG_NAME "TESRA"
#define QAPP_ORG_DOMAIN "tesra.org"
#define QAPP_APP_NAME_DEFAULT "TESRA-Qt"
#define QAPP_APP_NAME_TESTNET "TESRA-Qt-testnet"

#endif 
