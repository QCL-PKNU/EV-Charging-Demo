#include "KevDemoConfig.h"
#include "KevDemoMainWindow.h"
#include "ui_KevDemoMainWindow.h"

//////////////////////////////////////////////////
// Constructor & Destructor Definition
//////////////////////////////////////////////////

KevDemoMainWindow::KevDemoMainWindow(KevDemoEVCharger *pCharger, QWidget *pParent) :
    QMainWindow(pParent),
    m_pUi(new Ui::KevDemoMainWindow)
{
    //////////////////////////////////////////////////
    /// Electric vehicle charger configuration
    //////////////////////////////////////////////////

    if(pCharger == nullptr)
    {
        printf("Invalid electric vehicle charger error");
        exit(0);
    }

    m_pCharger = pCharger;

    //////////////////////////////////////////////////
    /// GUI configuration
    //////////////////////////////////////////////////

    m_pUi->setupUi(this);

    // initialize the main switched widget to display usage message
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CAMERA);
    m_pUi->sw_side->setCurrentIndex(SIDE_PANE_ID_AUTH);

    // connect the signals and slots of authentication buttons
    QObject::connect(m_pUi->bt_plug_in, SIGNAL(clicked()),
                     this, SLOT(slot_plugInButtonClicked()));
    QObject::connect(m_pUi->bt_clear, SIGNAL(clicked()),
                     this, SLOT(slot_clearButtonClicked()));

    // scroll to bottom when a log message is inserted in the log widget
    QObject::connect(m_pUi->lw_log->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                     m_pUi->lw_log, SLOT(scrollToBottom()));

    //////////////////////////////////////////////////
    /// authentication
    //////////////////////////////////////////////////

    m_nAuthState = KEV_STATE_AUTH_IDLE;

    //////////////////////////////////////////////////
    /// camera preview configuration
    //////////////////////////////////////////////////

    m_pCameraPreview = new KevDemoCameraPreview();

    // Connect the signals and slots for camera preview
    QObject::connect(m_pCameraPreview, SIGNAL(sig_receiveFrame(cv::Mat)),
                     this, SLOT(slot_receiveFrame(cv::Mat)));

    // start to display a camera preview
    if(m_pCameraPreview == nullptr || m_pCameraPreview->open(0) == false)
    {
        printLog("Failed to open a camera capture.");
        return;
    }

    //////////////////////////////////////////////////
    /// VLC decoder configuration (Threshold: 100)
    //////////////////////////////////////////////////

    int threshold = m_pUi->le_vlc_thres->text().toInt();
    int dataWidth = m_pUi->le_vlc_datawidth->text().toInt();
    int clockindex = m_pUi->le_vlc_clock->text().toInt();

    m_pVLCDecoder = new KevDemoVLCDecoder(KEV_VLC_DEC_MI);
    m_pVLCDecoder->setThreshold(threshold);
    m_pVLCDecoder->setDataWidth(dataWidth);
    m_pVLCDecoder->setClockIndex(clockindex);

    m_nFrameCount = 0;

    m_nVLCDecodedBits = 0;
    m_nVLCDecodedByte = 0;

    // VLC decoder signal handling
    QObject::connect(m_pVLCDecoder, SIGNAL(sig_printDebugMessage(QString)),
                     this, SLOT(slot_printDebugMessage(QString)));

    // VLC paramter changes
    QObject::connect(m_pUi->le_vlc_thres, SIGNAL(textChanged(const QString&)),
                     this, SLOT(slot_vlcThresholdChanged(QString)));
    QObject::connect(m_pUi->le_vlc_datawidth, SIGNAL(textChanged(const QString&)),
                     this, SLOT(slot_vlcDatawidthChanged(QString)));

    // Authentication
    QObject::connect(m_pVLCDecoder, SIGNAL(sig_performAuthentication(int)),
                     this, SLOT(slot_authenticationPerformed(int)));

    //////////////////////////////////////////////////
    /// VBC reader configuration (Baudrate: 203400)
    //////////////////////////////////////////////////

    m_pVBCReader = new KevDemoVBCReader();

    // VBC decoder signal handling
    QObject::connect(m_pVBCReader, SIGNAL(sig_printDebugMessage(QString)),
                     this, SLOT(slot_printDebugMessage(QString)));

    // Authentication
    QObject::connect(m_pVBCReader, SIGNAL(sig_performAuthentication(int)),
                     this, SLOT(slot_authenticationPerformed(int)));

    //////////////////////////////////////////////////
    /// Charging scheduler
    //////////////////////////////////////////////////

    QObject::connect(m_pCharger, SIGNAL(sig_printDebugMessage(QString)),
                     this, SLOT(slot_printDebugMessage(QString)));

    // scheduling
    QObject::connect(m_pCharger, SIGNAL(sig_chargerScheduled(int, double)),
                     this, SLOT(slot_schedulingPerformed(int, double)));

    //////////////////////////////////////////////////
    /// Charging type handling
    //////////////////////////////////////////////////

    m_pVehicle = nullptr;

    // AC charging
    QObject::connect(m_pUi->bt_ac_charging, SIGNAL(clicked()),
                     this, SLOT(slot_acChargingButtonClicked()));

    // DC charging
    QObject::connect(m_pUi->bt_dc_charging, SIGNAL(clicked()),
                     this, SLOT(slot_dcChargingButtonClicked()));

    //////////////////////////////////////////////////
    /// Charging menu handling
    //////////////////////////////////////////////////

    // Fixed charging amount
    QObject::connect(m_pUi->bt_fixed_charging, SIGNAL(clicked()),
                     this, SLOT(slot_fixedChargingButtonClicked()));

    // Fixed payment amount
    QObject::connect(m_pUi->bt_fixed_payment, SIGNAL(clicked()),
                     this, SLOT(slot_fixedPaymentButtonClicked()));

    // Fixed payment amount
    QObject::connect(m_pUi->bt_full_charging, SIGNAL(clicked()),
                     this, SLOT(slot_fullChargingButtonClicked()));

    //////////////////////////////////////////////////
    /// Fixed charging amount interface
    //////////////////////////////////////////////////

    QSignalMapper *sigmap1 = new QSignalMapper(this);

    // numbers
    QPushButton *bt_fcs[] = {m_pUi->bt_fc_0, m_pUi->bt_fc_1, m_pUi->bt_fc_2,
                             m_pUi->bt_fc_3, m_pUi->bt_fc_4, m_pUi->bt_fc_5,
                             m_pUi->bt_fc_6, m_pUi->bt_fc_7, m_pUi->bt_fc_8,
                             m_pUi->bt_fc_9, m_pUi->bt_fc_del, m_pUi->bt_fc_clr};

    for(int i = 0; i < 12; i++)
    {
        QObject::connect(bt_fcs[i], SIGNAL(clicked()), sigmap1, SLOT(map()));
        sigmap1->setMapping(bt_fcs[i], i);
    }

    QObject::connect(sigmap1, SIGNAL(mapped(int)),
                     this, SLOT(slot_fixedChargingKeypadButtonClicked(int)));

    // continue
    QObject::connect(m_pUi->bt_fc_continue, SIGNAL(clicked()),
                     this, SLOT(slot_fixedChargingContinueButtonClicked()));

    // cancel
    QObject::connect(m_pUi->bt_fc_cancel, SIGNAL(clicked()),
                     this, SLOT(slot_fixedChargingCancelButtonClicked()));

    //////////////////////////////////////////////////
    /// Fixed payment amount interface
    //////////////////////////////////////////////////

    QSignalMapper *sigmap2 = new QSignalMapper(this);

    // numbers
    QPushButton *bt_fps[] = {m_pUi->bt_fp_0, m_pUi->bt_fp_1, m_pUi->bt_fp_2,
                             m_pUi->bt_fp_3, m_pUi->bt_fp_4, m_pUi->bt_fp_5,
                             m_pUi->bt_fp_6, m_pUi->bt_fp_7, m_pUi->bt_fp_8,
                             m_pUi->bt_fp_9, m_pUi->bt_fp_del, m_pUi->bt_fp_clr};

    for(int i = 0; i < 12; i++)
    {
        QObject::connect(bt_fps[i], SIGNAL(clicked()), sigmap2, SLOT(map()));
        sigmap2->setMapping(bt_fps[i], i);
    }

    QObject::connect(sigmap2, SIGNAL(mapped(int)),
                     this, SLOT(slot_fixedPaymentKeypadButtonClicked(int)));

    // continue
    QObject::connect(m_pUi->bt_fp_continue, SIGNAL(clicked()),
                     this, SLOT(slot_fixedPaymentContinueButtonClicked()));

    // cancel
    QObject::connect(m_pUi->bt_fp_cancel, SIGNAL(clicked()),
                     this, SLOT(slot_fixedPaymentCancelButtonClicked()));

    //////////////////////////////////////////////////
    /// Charging time configuration interface
    //////////////////////////////////////////////////

    // up
    QSignalMapper *sigmap3 = new QSignalMapper(this);

    QPushButton *bt_ups[] = {m_pUi->bt_dd_up, m_pUi->bt_hh_up, m_pUi->bt_mm_up};

    for(int i = 0; i < 3; i++)
    {
        QObject::connect(bt_ups[i], SIGNAL(clicked()), sigmap3, SLOT(map()));
        sigmap3->setMapping(bt_ups[i], i);
    }

    QObject::connect(sigmap3, SIGNAL(mapped(int)),
                     this, SLOT(slot_chargingTimeUpButtonClicked(int)));

    // down
    QSignalMapper *sigmap4 = new QSignalMapper(this);

    QPushButton *bt_dws[] = {m_pUi->bt_dd_down, m_pUi->bt_hh_down, m_pUi->bt_mm_down};

    for(int i = 0; i < 3; i++)
    {
        QObject::connect(bt_dws[i], SIGNAL(clicked()), sigmap4, SLOT(map()));
        sigmap4->setMapping(bt_dws[i], i);
    }

    QObject::connect(sigmap4, SIGNAL(mapped(int)),
                     this, SLOT(slot_chargingTimeDownButtonClicked(int)));

    // continue
    QObject::connect(m_pUi->bt_charging_time_continue, SIGNAL(clicked()),
                     this, SLOT(slot_chargingTimeContinueButtonClicked()));

    // cancel
    QObject::connect(m_pUi->bt_charging_time_cancel, SIGNAL(clicked()),
                     this, SLOT(slot_chargingTimeCancelButtonClicked()));

    //////////////////////////////////////////////////
    /// Charging interface
    //////////////////////////////////////////////////

    QObject::connect(m_pUi->bt_charging_stop, SIGNAL(clicked()),
                     this, SLOT(slot_chargingStopButtonClicked()));

    QObject::connect(m_pUi->bt_charging_pay, SIGNAL(clicked()),
                     this, SLOT(slot_chargingPayButtonClicked()));

    QObject::connect(m_pUi->bt_charging_restart, SIGNAL(clicked()),
                     this, SLOT(slot_chargingRestartButtonClicked()));

    //////////////////////////////////////////////////
    /// Database initialization
    //////////////////////////////////////////////////

    m_pDatabase = new KevDemoDatabase();

    if(m_pDatabase->open(0) != KEV_SUCCESS)
    {
        printLog("Failed to open an authentication database.");
        return;
    }

    m_pDatabase->registerAuthInfo(0xAA, "46K 4502", QImage(":/res/img/user1.jpg"));
    m_pDatabase->registerAuthInfo(0xA2, "29D 3281", QImage(":/res/img/user2.png"));
    m_pDatabase->registerAuthInfo(0xDA, "04H 6609", QImage(":/res/img/user3.jpg"));
    m_pDatabase->registerAuthInfo(0xFF, "51A 9423", QImage(":/res/img/user4.jpg"));
}

KevDemoMainWindow::~KevDemoMainWindow()
{
    if(m_pCameraPreview != nullptr)
    {
        m_pCameraPreview->close();
        delete m_pCameraPreview;
    }

    if(m_pVLCDecoder != nullptr)
    {
        delete m_pVLCDecoder;
    }

    if(m_pDatabase != nullptr)
    {
        m_pDatabase->close();
        delete m_pDatabase;
    }

    delete m_pUi;
}

//////////////////////////////////////////////////
// Member Function Definition
//////////////////////////////////////////////////

/**
 * @brief This is a funciton to perform a user authentication.
 */
void
KevDemoMainWindow::performUserAuthentication()
{
#ifndef KEV_DUMMY_AUTHENTICATE
    // VBC
    if(m_pVBCReader == nullptr)
    {
        m_pVBCReader = new KevDemoVBCReader();
    }
    else if(m_pVBCReader->isRunning() == true)
    {
        m_pVBCReader->close();
    }

    // open vibration communication
    uint32_t threshold = m_pUi->le_vbc_thres->text().toUInt();
    uint32_t period = m_pUi->le_vbc_period->text().toUInt();

    m_pVBCReader->open(230400, threshold, period);
#else
    slot_authenticationPerformed(0xA2);
#endif
}

/**
 * @brief This function is used to display the user photo, which is indicated by an user ID.
 * @param nId user ID
 */
void
KevDemoMainWindow::displayUserPhoto(uint32_t nId)
{
    QImage img = m_pDatabase->getUserPhoto(nId);
    QSize imgSize = m_pUi->lb_user_photo->size();
    img = img.scaled(imgSize, Qt::KeepAspectRatio);
    m_pUi->lb_user_photo->setPixmap(QPixmap::fromImage(img));
}

/**
 * @brief This funciton prints the given string on the log pane.
 * @param rString a string to be logged
 */
void
KevDemoMainWindow::printLog(const QString rString)
{
    if(m_pUi->cb_log->isChecked())
    {
        m_pUi->lw_log->addItem(rString);
    }
}

//////////////////////////////////////////////////
// Slot Function Definition
//////////////////////////////////////////////////

/**
 * @brief This is a slot funciton to handle the signal when an plug-in button is clicked.
 */
void
KevDemoMainWindow::slot_plugInButtonClicked()
{
    if(m_pVBCReader != nullptr && m_pVBCReader->isRunning() == true)
    {
        m_pVBCReader->close();
    }

    // start to authenticate a car using VLC
    printLog("Car authentication starts ...");
    m_nAuthState = KEV_STATE_AUTH_CAR;

    // switch to a cam & authentication pane
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CAMERA);
    m_pUi->sw_side->setCurrentIndex(SIDE_PANE_ID_AUTH);
}

/**
 * @brief This is a slot function to clear the main window.
 */
void
KevDemoMainWindow::slot_clearButtonClicked()
{
    m_pUi->lb_user_photo->clear();
    m_pUi->le_car_id->clear();
}

/**
 * @brief This is a slot function to update the data width of VLC decoder.
 * @param rString a string indicating data width
 */
void
KevDemoMainWindow::slot_vlcDatawidthChanged(QString rString)
{
    int datawidth = rString.toInt();

    if(m_pVLCDecoder != nullptr)
    {
        m_pVLCDecoder->setDataWidth(datawidth);
    }
}

/**
 * @brief This is a slot function to update the threshold of VLC decoder.
 * @param a string indicating threshold value
 */
void
KevDemoMainWindow::slot_vlcThresholdChanged(QString rString)
{
    int threshold = rString.toInt();

    if(m_pVLCDecoder != nullptr)
    {
        m_pVLCDecoder->setThreshold(threshold);
    }
}

/**
 * @brief This is a slot function to perform AC charging.
 */
void
KevDemoMainWindow::slot_acChargingButtonClicked()
{
    if(m_pCharger == NULL)
    {
        printLog("Error - EVCharger::slot_acChargingButtonClicked with AC charging");
        return;
    }

    m_pCharger->setChargingType(KEV_CHARGE_AC);

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_MENU);
    m_pUi->le_charging_type->setText("AC");
}

/**
 * @brief This is a slot function to perform DC charging.
 */
void
KevDemoMainWindow::slot_dcChargingButtonClicked()
{
    if(m_pCharger == NULL)
    {
        printLog("Error - EVCharger::slot_acChargingButtonClicked with DC charging");
        return;
    }

    m_pCharger->setChargingType(KEV_CHARGE_DC);

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_MENU);
    m_pUi->le_charging_type->setText("DC");
}

/**
 * @brief This is a slot function to perform fixed charging.
 */
void
KevDemoMainWindow::slot_fixedChargingButtonClicked()
{   
    KevDemoError_t error = m_pCharger->setupCharging(KEV_CHARGE_FIXED_AMOUNT);
    if(error != KEV_SUCCESS)
    {
        printLog("Error - EVCharger::setupCharging with full charging");
        return;
    }

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_FIXED_CHARGING);
}

/**
 * @brief This is a slot function to perform fixed payment.
 */
void
KevDemoMainWindow::slot_fixedPaymentButtonClicked()
{
    KevDemoError_t error = m_pCharger->setupCharging(KEV_CHARGE_FIXED_PAYMENT);
    if(error != KEV_SUCCESS)
    {
        printLog("Error - EVCharger::setupCharging with full charging");
        return;
    }

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_FIXED_PAYMENT);
}

/**
 * @brief This is a slot function to perform full charging.
 */
void
KevDemoMainWindow::slot_fullChargingButtonClicked()
{
    KevDemoError_t error = m_pCharger->setupCharging(KEV_CHARGE_FULL);
    if(error != KEV_SUCCESS)
    {
        printLog("Error - EVCharger::setupCharging with full charging");
        return;
    }

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TIME);
    m_pUi->te_current_time->setDateTime(QDateTime::currentDateTime());
    m_pUi->te_target_time->setDateTime(QDateTime::currentDateTime());
    m_pUi->lcd_fixed_charging->display(0);
}


/**
 * @brief This is a slot function to input charging amount.
 * @param value the input value of pushed button
 */
void
KevDemoMainWindow::slot_fixedChargingKeypadButtonClicked(int value)
{
    int num = m_pUi->lcd_fixed_charging->intValue();

    switch(value)
    {
        case KEY_KEYPAD_DEL:
            m_pUi->lcd_fixed_charging->display(num / 10);
            break;
        case KEY_KEYPAD_CLR:
            m_pUi->lcd_fixed_charging->display(0);
            break;
        default:
            if(num <= 9999999)
            {
                m_pUi->lcd_fixed_charging->display(num * 10 + value);
            }
            break;
    }

    // update the remaining time for charging
    double chargingAmount = (double)m_pUi->lcd_fixed_charging->intValue();
    double chargingTime = chargingAmount / m_pCharger->getChargingKwPerMin();

    m_pUi->le_remaining_time->setText(QString().setNum((int)chargingTime));
}

/**
 * @brief This is a slot function to continue the charging with fixed charging amount.
 */
void
KevDemoMainWindow::slot_fixedChargingContinueButtonClicked()
{
    double chargingAmount = (double)m_pUi->lcd_fixed_charging->intValue();

    KevDemoError_t error = m_pCharger->setupCharging(KEV_CHARGE_FIXED_AMOUNT, chargingAmount);
    if(error != KEV_SUCCESS)
    {
        printLog("Error - EVCharger::setupCharging with fixed charging amount");
        return;
    }

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TIME);
    m_pUi->te_current_time->setDateTime(QDateTime::currentDateTime());
    m_pUi->te_target_time->setDateTime(QDateTime::currentDateTime());
    m_pUi->lcd_fixed_charging->display(0);
}

/**
 * @brief This is a slot function to cancel the charging with fixed charging amount.
 */
void
KevDemoMainWindow::slot_fixedChargingCancelButtonClicked()
{
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TYPE);
    m_pUi->le_remaining_time->setText(QString().setNum(0));
    m_pUi->lcd_fixed_charging->display(0);
    m_pUi->le_charging_type->setText("");
}

/**
 * @brief This is a slot function to input payment amount.
 * @param value the input value of pushed button
 */
void
KevDemoMainWindow::slot_fixedPaymentKeypadButtonClicked(int value)
{
    int num = m_pUi->lcd_fixed_payment->intValue();

    switch(value)
    {
        case KEY_KEYPAD_DEL:
            m_pUi->lcd_fixed_payment->display(num / 10);
            break;
        case KEY_KEYPAD_CLR:
            m_pUi->lcd_fixed_payment->display(0);
            break;
        default:
            if(num <= 9999999)
            {
                m_pUi->lcd_fixed_payment->display(num * 10 + value);
            }
            break;
    }

    // update the remaining time for charging
    double chargingAmount = (double)m_pUi->lcd_fixed_payment->intValue() / m_pCharger->getChargingCostPerKw();
    double chargingTime = chargingAmount / m_pCharger->getChargingKwPerMin();

    m_pUi->le_remaining_time->setText(QString().setNum((int)chargingTime));
}

/**
 * @brief This is a slot function to continue the charging with fixed payment amount.
 */
void
KevDemoMainWindow::slot_fixedPaymentContinueButtonClicked()
{
    double paymentAmount = (double)m_pUi->lcd_fixed_payment->intValue();

    KevDemoError_t error = m_pCharger->setupCharging(KEV_CHARGE_FIXED_PAYMENT, paymentAmount);
    if(error != KEV_SUCCESS)
    {
        printLog("Error - EVCharger::setupCharging with fixed payment amount");
        return;
    }

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TIME);
    m_pUi->te_current_time->setDateTime(QDateTime::currentDateTime());
    m_pUi->te_target_time->setDateTime(QDateTime::currentDateTime());
    m_pUi->lcd_fixed_payment->display(0);
}

/**
 * @brief This is a slot function to cancel the charging with fixed payment amount.
 */
void
KevDemoMainWindow::slot_fixedPaymentCancelButtonClicked()
{
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TYPE);
    m_pUi->le_remaining_time->setText(QString().setNum(0));
    m_pUi->lcd_fixed_payment->display(0);
    m_pUi->le_charging_type->setText("");
}

/**
 * @brief This is a slot function to increase the target charging time.
 * @param value time indicator (0: dd, 1: HH, 2: mm)
 */
void
KevDemoMainWindow::slot_chargingTimeUpButtonClicked(int value)
{
    QDateTime dateTime = m_pUi->te_target_time->dateTime();

    switch (value) {
        case 0: // dd
            dateTime = dateTime.addDays(1);
            break;
        case 1: // HH : 60 x 60 secs
            dateTime = dateTime.addSecs(3600);
            break;
        case 2: // mm : 60 secs
            dateTime = dateTime.addSecs(60);
        default:
            break;
    }

    m_pUi->te_target_time->setDateTime(dateTime);
}

/**
 * @brief This is a slot function to decrease the target charging time.
 * @param value time indicator (0: dd, 1: HH, 2: mm)
 */
void
KevDemoMainWindow::slot_chargingTimeDownButtonClicked(int value)
{
    QDateTime dateTime = m_pUi->te_target_time->dateTime();

    switch (value) {
        case 0: // dd
            dateTime = dateTime.addDays(-1);
            break;
        case 1: // HH : 60 x 60 secs
            dateTime = dateTime.addSecs(-3600);
            break;
        case 2: // mm : 60 secs
            dateTime = dateTime.addSecs(-60);
        default:
            break;
    }

    m_pUi->te_target_time->setDateTime(dateTime);
}

void
KevDemoMainWindow::slot_chargingTimeContinueButtonClicked()
{
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING);

    QTime targetTime = m_pUi->te_target_time->time();

    if(m_pCharger->startCharging(targetTime) != KEV_SUCCESS)
    {
        printLog("Error - EVCharger::startCharging");
        return;
    }

    m_pUi->te_charged_time->setTime(QTime(0, 0, 0));
}

void
KevDemoMainWindow::slot_chargingTimeCancelButtonClicked()
{
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TYPE);

    m_pUi->le_remaining_time->setText(QString().setNum(0));
    m_pUi->te_current_time->setTime(QTime::currentTime());
    m_pUi->te_target_time->setTime(QTime::currentTime());
    m_pUi->le_charging_type->setText("");
}

/**
 * @brief This is a slot function to stop the charging.
 */
void
KevDemoMainWindow::slot_chargingStopButtonClicked()
{
    printLog("Electric charging is stopped.");
    m_pCharger->stopCharging();
}

/**
 * @brief This is a slot function to pay for the charging.
 */
void
KevDemoMainWindow::slot_chargingPayButtonClicked()
{
    if(m_pCharger->isRunning())
    {
        printLog("Please stop charging first.");
        return;
    }

    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_FINISH);
}

void
KevDemoMainWindow::slot_chargingRestartButtonClicked()
{
    // return to the start
    m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_DEFAULT);
    m_pUi->sw_side->setCurrentIndex(SIDE_PANE_ID_AUTH);

    // clear authentication information
    m_pUi->lb_user_photo->setPixmap(QPixmap());
    m_pUi->le_car_id->setText(QString());
}

/**
 * @brief This is a slot funciton to handle the signal of receiving a frame.
 * @param rPreviewFrame a matrix of camera preview frame
 */
void
KevDemoMainWindow::slot_receiveFrame(cv::Mat rPreviewFrame)
{
    m_nFrameCount++;

    cv::Mat frame;

    int w = m_pUi->lb_cam->width();
    int h = m_pUi->lb_cam->height();

#ifndef KEV_DUMMY_AUTHENTICATE
    int numRecvBits = 0;

    if(m_nAuthState == KEV_STATE_AUTH_CAR)
    {
        numRecvBits = 8;
    }
    else if(m_nAuthState == KEV_STATE_INFO_CAR)
    {
        numRecvBits = 16;
    }
#endif

    cv::resize(rPreviewFrame, frame, cv::Size(w, h));

    // skip if the current pane is not for camera preview
    if(m_nAuthState == KEV_STATE_AUTH_CAR || m_nAuthState == KEV_STATE_INFO_CAR)
    {
#ifndef KEV_DUMMY_AUTHENTICATE
        // perform VLC decoding
        std::vector<int> decodedBits;

        if(m_pVLCDecoder->decode(frame, decodedBits) != KEV_SUCCESS)
        {
            return;
        }

        for(int decodedBit : decodedBits)
        {
            m_nVLCDecodedByte = (m_nVLCDecodedByte << 1) | decodedBit;

            if(++m_nVLCDecodedBits >= numRecvBits)
            {
                QString str("> Decoded bits: ");
                str.append(QString::number(m_nVLCDecodedByte, 16));
                printLog(str);

                // finish to authenticate a car using VLC
                slot_authenticationPerformed(m_nVLCDecodedByte);

                m_nVLCDecodedByte = 0;
                m_nVLCDecodedBits = 0;
            }
        }
#else
        // YOUNGSUN - FOR DEMO
        if(m_nAuthState == KEV_STATE_AUTH_CAR)
        {
            slot_authenticationPerformed(0xA2);
        }
        else if(m_nAuthState == KEV_STATE_INFO_CAR)
        {
            slot_authenticationPerformed((246 << 8) | 78);
        }

        QThread::sleep(1);
#endif
    }

    // build a image
    QImage img = QImage((const unsigned char *)frame.data,
                        frame.cols, frame.rows, QImage::Format_Indexed8);

    m_pUi->lb_cam->setPixmap(QPixmap::fromImage(img));
}

/**
 * @brief This is a slot function to print the given string on a log window.
 * @param rString a string to be printed on a log window.
 */
void
KevDemoMainWindow::slot_printDebugMessage(const QString rString)
{
    printLog(rString);
}

/**
 * @brief This is a slot function to perform authentication with auth. information.
 * @param nAuthInfo auth. information
 */
void
KevDemoMainWindow::slot_authenticationPerformed(int nAuthInfo)
{
    switch(m_nAuthState)
    {
        case KEV_STATE_AUTH_CAR:
        {
            printLog("Car authentication completes.");
            QString evNumber = m_pDatabase->getEVNumber(nAuthInfo);
            m_pUi->le_car_id->setText(evNumber);

            // create a model for an electric vehicle
            m_pVehicle = new KevDemoEVehicle((uint32_t)nAuthInfo);
            m_pCharger->setEVehicle(m_pVehicle);

            // start to user authentication using VBC
            m_nAuthState = KEV_STATE_AUTH_USER;
            performUserAuthentication();

            break;
        }
        case KEV_STATE_AUTH_USER:
        {
            printLog("User authentication completes.");
            displayUserPhoto(nAuthInfo);

            if(m_pVBCReader != nullptr && m_pVBCReader->isRunning() == true)
            {
                m_pVBCReader->close();
            }

            // update the electric vehicle model
            if(m_pVehicle != nullptr)
            {
                m_pVehicle->setUserId(nAuthInfo);
            }

            // switch to a cam pane to receive car information
            m_nAuthState = KEV_STATE_INFO_CAR;
            m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CAMERA);

            break;
        }
        case KEV_STATE_INFO_CAR:
        {
            printLog("Car information received.");
            m_nAuthState = KEV_STATE_AUTH_IDLE;

            double chargingAmount   = (nAuthInfo & 0x00FF);
            double chargingCapacity = (nAuthInfo & 0xFF00) >> 8;

            chargingAmount *= 100;
            chargingCapacity *= 100;

            // update the electric vehicle model and connect to an electric charger
            if(m_pVehicle != nullptr)
            {
                m_pVehicle->setChargingAmount(chargingAmount);
                m_pVehicle->setChargingCapacity(chargingCapacity);
            }

            // update charging ratio
            int chargingRatio = (int)(100 * chargingAmount / chargingCapacity);
            m_pUi->pb_charging_ratio->setValue(chargingRatio);

            // update charging amount
            m_pUi->le_charging_amount->setText(QString().setNum(chargingAmount, 'f', 1));

            // display the electricity cost per kW
            m_pUi->le_charging_cost->setText(QString().setNum(m_pCharger->getChargingCostPerKw(), 'f', 0));

            // switch to a charging menu & charge status pane
            m_pUi->sw_main->setCurrentIndex(MAIN_PANE_ID_CHARGING_TYPE);
            m_pUi->sw_side->setCurrentIndex(SIDE_PANE_ID_CHARGING);
            m_pUi->le_charging_type->setText("");
            break;
        }
        default:
            break;
    }
}

/**
 * @brief This is a slot function to perform charger scheduling.
 * @param nScheduleInfo charger scheduling information
 * @param nChargedAmount charging amount
 */
void
KevDemoMainWindow::slot_schedulingPerformed(int nScheduleInfo, double nChargedAmount)
{
    // currently just charge the vehicle without scheduling
    if(m_pCharger == nullptr)
    {
        return;
    }

    double chargingRatio = m_pVehicle->getChargingRatio();
    double chargingAmount = m_pVehicle->getChargingAmount();

    double remainingAmount = m_pCharger->getChargingTargetAmount() - chargingAmount;
    double remainingTime = remainingAmount / m_pCharger->getChargingKwPerMin();

    m_pUi->pb_charging_ratio->setValue(chargingRatio);
    m_pUi->le_charging_amount->setText(QString().setNum(chargingAmount, 'f', 1));
    m_pUi->le_remaining_time->setText(QString().setNum((int)remainingTime));

    // full
    if(nScheduleInfo == 0)
    {
        // turn on the full led
        m_pUi->lb_full_signal->setVisible(true);
        m_pUi->lb_power_signal->setVisible(false);
        m_pUi->lb_stop_signal->setVisible(false);
    }
    // power
    else if(nScheduleInfo == 1)
    {
        m_pUi->lcd_charged_amount->display(QString().setNum(nChargedAmount, 'f', 1));
        m_pUi->lcd_charged_payment->display(QString().setNum(nChargedAmount * m_pCharger->getChargingCostPerKw(), 'f', 0));

        // update the spent time for charging
        QTime time = m_pUi->te_charged_time->time();
        time = time.addSecs(1);
        m_pUi->te_charged_time->setTime(time);

        // turn on the power led
        m_pUi->lb_full_signal->setVisible(false);
        m_pUi->lb_power_signal->setVisible(true);
        m_pUi->lb_stop_signal->setVisible(false);
    }
    // stop
    else if(nScheduleInfo == 2)
    {
        // turn on the stop led
        m_pUi->lb_full_signal->setVisible(false);
        m_pUi->lb_power_signal->setVisible(false);
        m_pUi->lb_stop_signal->setVisible(true);
    }
}
