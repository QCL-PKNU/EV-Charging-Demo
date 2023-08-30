#ifndef _KEV_DEMO_EVCHARGER_H_
#define _KEV_DEMO_EVCHARGER_H_

#include "KevDemoConfig.h"
#include "KevDemoEVehicle.h"
#include "KevDemoServerConn.h"

#define KEV_CHARGE_AC 0
#define KEV_CHARGE_DC 1

#define KEV_CHARGE_IDLE            0
#define KEV_CHARGE_FIXED_AMOUNT    1
#define KEV_CHARGE_FIXED_PAYMENT   2
#define KEV_CHARGE_FULL            3

#define KEV_STATE_CHARGER_IDLE     0
#define KEV_STATE_CHARGER_BUSY     1

/**
 * @brief a class for presenting an electric vehicle charger
 */
class KevDemoEVCharger : public QThread
{
    Q_OBJECT

private:

    // charger thread
    QThread *m_pThread;

    // indicate whether the charger is still running or not
    bool m_bIsRunning;

    // charger scheduling server
    KevDemoServerConn *m_pServerConn;

    // charging start time that the charing is actually started
    QTime m_rChargingStartTime;
    // charging final time that the charing is actually performed
    QTime m_rChargingFinalTime;
    // charging target time that the user wants to finish the charging
    QTime m_rChargingTargetTime;

    // charging types that the user can select
    // 1. AC
    // 2. DC
    uint8_t m_nChargingType;

    // charging menus that the user can select
    // 1. specific amount - ordered to charged as much as the specific amount
    // 2. specific cost - ordered to charged as much as the specific cost
    // 3. full - ordered to be fully charged
    uint8_t m_nChargingMenu;

    // charging state
    uint8_t m_nChargingState;

    // charging target amount
    double m_nChargingTargetAmount;

    // charging cost per 1kw
    double m_nChargingCostPerKw;

    // charging speed (kw / min)
    double m_nChargingKwPerMin;

    // a registered electric vehicle
    KevDemoEVehicle *m_pEVehicle;

public:

    explicit KevDemoEVCharger(KevDemoEVehicle *pEVehicle = NULL);
    virtual ~KevDemoEVCharger();

    // accessor & mutator
    inline void setEVehicle(KevDemoEVehicle *pEVehicle) { m_pEVehicle = pEVehicle; }
    inline KevDemoEVehicle *getEVehicle()               { return m_pEVehicle;      }

    inline void setChargingType(uint8_t nChargingType)   { m_nChargingType = nChargingType; }
    inline uint8_t getChargingType()                    { return m_nChargingType;          }

    inline void setCharingMenu(uint8_t nChargingMenu)   { m_nChargingMenu = nChargingMenu; }
    inline uint8_t getChargingMenu()                    { return m_nChargingMenu;          }

    inline QTime &getChargingStartTime()                  { return m_rChargingStartTime;     }
    inline QTime &getChargingFinalTime()                  { return m_rChargingFinalTime;     }

    inline void setChargingCostPerKw(double nCostPerKw)    { m_nChargingCostPerKw = nCostPerKw;  }
    inline double getChargingCostPerKw()                   { return m_nChargingCostPerKw;        }

    inline void setChargingKwPerMin(double nKwPerMin)      { m_nChargingKwPerMin = nKwPerMin; }
    inline double getChargingKwPerMin()                    { return m_nChargingKwPerMin;      }

    inline double getChargingTargetAmount()               { return m_nChargingTargetAmount; }
    inline uint8_t getChargingState()                     { return m_nChargingState; }

    /**
     * @brief This function returns whether the reader is now running or not.
     * @return true if the reader is operating, otherwise false.
     */
    inline bool isRunning() { return m_bIsRunning; }

    // member functions
    KevDemoError_t setupCharging(uint8_t nChargingMenu, double nChargingInfo = 0);
    KevDemoError_t startCharging(QTime rChargingTime);
    void stopCharging();

    int32_t getElapsedChargingTime();

    // server connection
    KevDemoError_t connectToServer(KevDemoServerConn *pServerConn);

    // thread
    void run();

signals:

    void sig_chargerScheduled(int nScheduleInfo, double nChargedAmount);

    void sig_printDebugMessage(QString rString);

private:

    void resetCharging();

    double performCharging(double nChargingAmount);
};

#endif // _KEV_DEMO_EVCHARGER_H_
