#ifndef _KEV_DEMO_EVEHICLE_H_
#define _KEV_DEMO_EVEHICLE_H_

#include "KevDemoConfig.h"

/**
 * @brief a class for presenting an electric vehicle
 */
class KevDemoEVehicle
{
private:

    // car ID
    uint32_t m_nCarId;

    // user ID
    uint32_t m_nUserId;

    // battery charging capacity
    double m_nCharingCapacity;

    // current charging amount
    double m_nCharingAmount;

public:

    explicit KevDemoEVehicle(uint32_t nCarId);
    explicit KevDemoEVehicle(uint32_t nCarId, uint32_t nUserId);
    virtual ~KevDemoEVehicle();

    // accessor & mutator
    inline uint32_t getCarId()                          { return m_nCarId;     }

    inline void setUserId(uint32_t nUserId)             { m_nUserId = nUserId; }
    inline uint32_t getUserId()                         { return m_nUserId;    }

    inline void setChargingCapacity(double nCapacity) { m_nCharingCapacity = nCapacity; }
    inline double getChargingCapacity()               { return m_nCharingCapacity;      }

    inline void setChargingAmount(double nAmount)     { m_nCharingAmount = nAmount; }
    inline double getChargingAmount()                 { return m_nCharingAmount;    }

    inline bool isFullyCharged()                        { return m_nCharingCapacity == m_nCharingAmount; }
    inline uint32_t getChargingRatio()                  { return (int)m_nCharingAmount / m_nCharingCapacity * 100; }

    // member functions
    double performCharging(double nChargingAmount, double nTargetChargingAmount);
};

#endif // _KEV_DEMO_EVEHICLE_H_
