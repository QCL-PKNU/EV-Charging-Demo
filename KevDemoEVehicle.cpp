#include "KevDemoEVehicle.h"

/**
 * @brief a constructor of a electric vehicle
 * @param nCarId electric vehicle ID
 */
KevDemoEVehicle::KevDemoEVehicle(uint32_t nCarId)
{
    m_nCarId = nCarId;

    // initialize battery charging
    m_nCharingCapacity = 0;
    m_nCharingAmount = 0;
}

/**
 * @brief a constructor of a electric vehicle
 * @param nCarId electric vehicle ID
 * @param nUserId user ID
 */
KevDemoEVehicle::KevDemoEVehicle(uint32_t nCarId, uint32_t nUserId)
{
    m_nCarId = nCarId;
    m_nUserId = nUserId;

    // initialize battery charging
    m_nCharingCapacity = 0;
    m_nCharingAmount = 0;
}

/**
 * @brief a destructor of a electric vehicle
 */
KevDemoEVehicle::~KevDemoEVehicle()
{

}

/**
 * @brief This function performs charging as much as the given amount of electricity.
 * @param nChargingAmount amount of electricity to be charged
 * @param nTargetChargingAmount target charging amount
 */
double
KevDemoEVehicle::performCharging(double nChargingAmount, double nTargetChargingAmount)
{
    double oldChargingAmount = m_nCharingAmount;

    m_nCharingAmount += nChargingAmount;

    if(m_nCharingAmount > m_nCharingCapacity)
    {
        m_nCharingAmount = m_nCharingCapacity;
    }
    else if(m_nCharingAmount > nTargetChargingAmount)
    {
        m_nCharingAmount = nTargetChargingAmount;
    }

    return m_nCharingAmount - oldChargingAmount;
}
