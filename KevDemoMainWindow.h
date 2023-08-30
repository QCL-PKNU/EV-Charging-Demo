#ifndef _KEV_DEMO_MAIN_WINDOW_H_
#define _KEV_DEMO_MAIN_WINDOW_H_

#include "KevDemoConfig.h"
#include "KevDemoDatabase.h"
#include "KevDemoVLCDecoder.h"
#include "KevDemoVBCReader.h"
#include "KevDemoCameraPreview.h"
#include "KevDemoEVCharger.h"
#include "KevDemoEVehicle.h"

// switched widget panes
#define MAIN_PANE_ID_DEFAULT        0
#define MAIN_PANE_ID_CAMERA         (MAIN_PANE_ID_DEFAULT+1)
#define MAIN_PANE_ID_CHARGING_TYPE  (MAIN_PANE_ID_CAMERA+1)
#define MAIN_PANE_ID_CHARGING_MENU  (MAIN_PANE_ID_CHARGING_TYPE+1)
#define MAIN_PANE_ID_FIXED_CHARGING (MAIN_PANE_ID_CHARGING_MENU+1)
#define MAIN_PANE_ID_FIXED_PAYMENT  (MAIN_PANE_ID_FIXED_CHARGING+1)
#define MAIN_PANE_ID_CHARGING_TIME  (MAIN_PANE_ID_FIXED_PAYMENT+1)
#define MAIN_PANE_ID_CHARGING       (MAIN_PANE_ID_CHARGING_TIME+1)
#define MAIN_PANE_ID_FINISH         (MAIN_PANE_ID_CHARGING+1)

#define SIDE_PANE_ID_CONFIG   0
#define SIDE_PANE_ID_AUTH     (SIDE_PANE_ID_CONFIG+1)
#define SIDE_PANE_ID_CHARGING (SIDE_PANE_ID_AUTH+1)

// authentication state
#define KEV_STATE_AUTH_IDLE 0
#define KEV_STATE_AUTH_CAR  (KEV_STATE_AUTH_IDLE+1)
#define KEV_STATE_AUTH_USER (KEV_STATE_AUTH_CAR+1)
#define KEV_STATE_INFO_CAR  (KEV_STATE_AUTH_USER+1)

// keypad
#define KEY_KEYPAD_DEL 10
#define KEY_KEYPAD_CLR 11

namespace Ui {
class KevDemoMainWindow;
}

class KevDemoMainWindow : public QMainWindow
{
    Q_OBJECT

private:

    // UI object
    Ui::KevDemoMainWindow *m_pUi;

    // Camera preview
    KevDemoCameraPreview *m_pCameraPreview;

    // VLC decoder
    KevDemoVLCDecoder *m_pVLCDecoder;
    uint32_t m_nVLCRequiredBits;
    uint32_t m_nVLCDecodedBits;
    uint32_t m_nVLCDecodedByte;

    // VBC reader
    KevDemoVBCReader *m_pVBCReader;

    // VLC frame counter
    uint32_t m_nFrameCount;

    // current state
    uint32_t m_nAuthState;

    // database
    KevDemoDatabase *m_pDatabase;

    // electric vehicle & charger
    KevDemoEVehicle *m_pVehicle;
    KevDemoEVCharger *m_pCharger;

public:

    explicit KevDemoMainWindow(KevDemoEVCharger *pCharger, QWidget *pParent = 0);
    virtual ~KevDemoMainWindow();

public slots:

    // Slots for plug-in button
    void slot_plugInButtonClicked();

    // Slots for clearing GUI
    void slot_clearButtonClicked();

    // Slots for authentication information
    void slot_authenticationPerformed(int nAuthInfo);

    // Slots for charging scheduling
    void slot_schedulingPerformed(int nScheduleInfo, double nChargedAmount);

    // Slots for VLC parameter changes
    void slot_vlcThresholdChanged(QString rString);
    void slot_vlcDatawidthChanged(QString rString);

    // Slots for charing types
    void slot_acChargingButtonClicked();
    void slot_dcChargingButtonClicked();

    // Slots for charing menus
    void slot_fixedChargingButtonClicked();
    void slot_fixedPaymentButtonClicked();
    void slot_fullChargingButtonClicked();

    // Slots for the pane to input charging amount
    void slot_fixedChargingKeypadButtonClicked(int value);
    void slot_fixedChargingContinueButtonClicked();
    void slot_fixedChargingCancelButtonClicked();

    // Slots for the pane to input payment amount
    void slot_fixedPaymentKeypadButtonClicked(int value);
    void slot_fixedPaymentContinueButtonClicked();
    void slot_fixedPaymentCancelButtonClicked();

    // Slots for setting charging time
    void slot_chargingTimeUpButtonClicked(int value);
    void slot_chargingTimeDownButtonClicked(int value);
    void slot_chargingTimeContinueButtonClicked();
    void slot_chargingTimeCancelButtonClicked();

    // Slots for the pane to charge
    void slot_chargingRestartButtonClicked();
    void slot_chargingStopButtonClicked();
    void slot_chargingPayButtonClicked();

    // Slots for a camera preview
    void slot_receiveFrame(cv::Mat rPreviewFrame);

    // Slots for pring a message
    void slot_printDebugMessage(const QString rString);

private:

    // perform user authentication using VBC
    void performUserAuthentication();

    // display user photo indicated by the given ID
    void displayUserPhoto(uint32_t nId);

    // print log message
    void printLog(const QString rString);
};

#endif // _KEV_DEMO_MAIN_WINDOW_H_
