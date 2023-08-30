#include "KevDemoEVCharger.h"

/**
 * @brief a constructor of an electric vehicle charger
 * @param pEVehicle an electric vehicle to be charged
 */
KevDemoEVCharger::KevDemoEVCharger(KevDemoEVehicle *pEVehicle)
{
    m_pEVehicle = pEVehicle;

    m_pServerConn = nullptr;
    m_pThread = nullptr;

    m_nChargingCostPerKw = 0;
    m_nChargingState = KEV_STATE_CHARGER_IDLE;
    m_nChargingType = KEV_CHARGE_IDLE;

    m_nChargingTargetAmount = 0;

    m_rChargingStartTime.start();
    m_rChargingFinalTime.start();
}

/**
 * @brief a destructor of an electric vehicle charger
 */
KevDemoEVCharger::~KevDemoEVCharger()
{
    if(m_pServerConn != nullptr) delete m_pServerConn;
    if(m_pEVehicle != nullptr) delete m_pEVehicle;
    if(m_pThread != nullptr) delete m_pThread;
}

/**
 * @brief This function is used to reset the charging.
 * @return error information
 */
void
KevDemoEVCharger::resetCharging()
{
    m_pEVehicle = NULL;
    m_pThread = NULL;

    m_nChargingCostPerKw = 0;
    m_nChargingState = KEV_STATE_CHARGER_IDLE;
    m_nChargingType = KEV_CHARGE_IDLE;

    m_nChargingTargetAmount = 0;

    m_rChargingStartTime.restart();
    m_rChargingFinalTime.restart();
}

/**
 * @brief This function is used to setup electric charging.
 * @param nChargingMenu charging menu (amount, cost, full, etc.)
 * @param nChargingInfo information to configure the charging target (charging amount or cost)
 * @return error information
 */
KevDemoError_t
KevDemoEVCharger::setupCharging(uint8_t nChargingMenu, double nChargingInfo)
{
    double chargingAmount = 0;
    double chargingCapacity = 0;

    // Check whether an electric vehicle is connected to be charged or not.
    if(m_pEVehicle == NULL)
    {
        return KEV_ERROR_EV_NOT_CONNECTED;
    }

    m_nChargingState = KEV_STATE_CHARGER_BUSY;
    m_nChargingMenu = nChargingMenu;

    chargingCapacity = m_pEVehicle->getChargingCapacity();

    // Update the target charging amount to be charged further
    switch(nChargingMenu)
    {
        case KEV_CHARGE_FULL:
        {
            chargingAmount = chargingCapacity - m_pEVehicle->getChargingAmount();
            break;
        }
        case KEV_CHARGE_FIXED_AMOUNT:
        {
            chargingAmount = nChargingInfo;
            break;
        }
        case KEV_CHARGE_FIXED_PAYMENT:
        {
            chargingAmount = nChargingInfo/m_nChargingCostPerKw;
            break;
        }
        default:
            break;
    }

    m_nChargingTargetAmount = m_pEVehicle->getChargingAmount() + chargingAmount;

    if(m_nChargingTargetAmount > chargingCapacity)
    {
        m_nChargingTargetAmount = chargingCapacity;
    }

    return KEV_SUCCESS;
}

/**
 * @brief This function is used to start electric charging.
 * @param rChargingTime charging time
 * @return error information
 */
KevDemoError_t
KevDemoEVCharger::startCharging(QTime rChargingTime)
{
    // Initialize the start and finish time for charging
    m_rChargingStartTime.restart();
    m_rChargingFinalTime.restart();

    // Update the charging target time
    m_rChargingTargetTime = rChargingTime;

    // Start to run the charging thread
    m_bIsRunning = true;
    this->start();

    return KEV_SUCCESS;
}

/**
 * @brief This function is used to stop electric charging.
 */
void
KevDemoEVCharger::stopCharging()
{
    m_bIsRunning = false;
}

/**
 * @brief This function performs electric charging as much as the given amount.
 * @param nChargingAmount charging amount to be charged at once
 * @return actually charged amount
 */
double
KevDemoEVCharger::performCharging(double nChargingAmount)
{
    double chargedAmount;

    // Check whether an electric vehicle is connected to be charged or not.
    if(m_pEVehicle == NULL)
    {
        return -1;
    }

    // Check whether charging has been already started or not.
    if(m_nChargingState == KEV_STATE_CHARGER_IDLE)
    {
        return -2;
    }

    // Performs charging as much as the given amount
    chargedAmount = m_pEVehicle->performCharging(nChargingAmount, m_nChargingTargetAmount);

    if(chargedAmount != 0)
    {
        // Update the final time of charging
        m_rChargingFinalTime.restart();
    }

    return chargedAmount;
}

/**
 * @brief This function returns an elapsed time for charging.
 * @return elapsed time (msec) from the start charging time to now
 */
int32_t
KevDemoEVCharger::getElapsedChargingTime()
{
    return m_rChargingStartTime.msecsTo(m_rChargingFinalTime);
}

/**
 * @brief This function is reserved to establish the connection to a scheduling server.
 * @param pServerConn a server connection
 * @return error information
 */
KevDemoError_t
KevDemoEVCharger::connectToServer(KevDemoServerConn *pServerConn)
{
    if(pServerConn == NULL)
    {
        return KEV_ERROR_INVALID_ARGUMENTS;
    }

    m_pServerConn = pServerConn;

    return KEV_SUCCESS;
}

/**
 * @brief This function is a slot function for the charging thread.
 */
void
KevDemoEVCharger::run()
{
    double totalChargedAmount = 0;

    while(m_bIsRunning == true)
    {
#if 0   // reserved for the next year
        if(m_pServerConn != nullptr)
        {
            uint8_t buf[16];
            int nread = m_pServerConn->read(buf, 16);

            // decode the read data and schedule the electric vehicle charger
        }
#endif

        double chargedAmount = performCharging(m_nChargingKwPerMin / 60);

        // error
        if(chargedAmount < 0)
        {
            emit sig_printDebugMessage("Error - KevDemoEVCharger::run()");
            return;
        }
        // full or stop
        else if(chargedAmount == 0)
        {
            m_bIsRunning = false;

            if(m_pEVehicle->getChargingAmount() == m_pEVehicle->getChargingCapacity())
            {
                // full
                emit sig_chargerScheduled(0, 0);
            }
            else
            {
                // stop
                emit sig_chargerScheduled(2, 0);
            }
        }
        // power
        else
        {
            totalChargedAmount += chargedAmount;

            m_bIsRunning = true;
            emit sig_chargerScheduled(1, totalChargedAmount);
        }

        sleep(1);
    }
}
