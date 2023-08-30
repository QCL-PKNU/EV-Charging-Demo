#ifndef _KEV_DEMO_CONFIG_H_
#define _KEV_DEMO_CONFIG_H_

//////////////////////////////////////////////////
// Header Inclusion
//////////////////////////////////////////////////

#include <QTime>
#include <QTimer>
#include <QImage>
#include <QQueue>
#include <QObject>
#include <QThread>
#include <QResource>
#include <QMainWindow>
#include <QSqlDatabase>
#include <QSignalMapper>

#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>

//////////////////////////////////////////////////
// Macro Definition
//////////////////////////////////////////////////

//#define KEV_VLC_DEC_DEBUG_ENABLE

//#define KEV_VBC_SERIAL_ENABLE

#define KEV_DUMMY_AUTHENTICATE

//////////////////////////////////////////////////
// Data Type Definition
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// Error Type Definition
//////////////////////////////////////////////////

typedef enum KevDemoError {

    KEV_SUCCESS = 0,

    // general errors
    KEV_ERROR_INVALID_ARGUMENTS,

    // for VLC
    KEV_ERROR_UNKNOWN_VLC_DECODER,
    KEV_ERROR_UNKNOWN_VLC_STATE,
    KEV_ERROR_VLC_NOT_OPENED,

    // for VBC
    KEV_ERROR_UNKNOWN_VBC_STATE,
    KEV_ERROR_VBC_NOT_OPENED,
    KEV_ERROR_UNKNOWN_VBC_DATAWIDTH,

    // for electric charging
    KEV_ERROR_ALREADY_CHARGING,
    KEV_ERROR_NOT_CHARGING,
    KEV_ERROR_EV_NOT_CONNECTED,
    KEV_ERRRO_UNKNOWN_CHARGE_TYPE,

} KevDemoError_t;


#endif // _KEV_DEMO_CONFIG_H_
