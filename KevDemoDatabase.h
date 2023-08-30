#ifndef _KEV_DEMO_DATABASE_H_
#define _KEV_DEMO_DATABASE_H_

#include "KevDemoConfig.h"

/**
 * @brief a class for supporting a database of authentication information
 */
class KevDemoDatabase
{
private:
    // storage for user photo
    std::map<uint32_t, QImage> m_rUserPhotoDB;
    std::map<uint32_t, QString> m_rEVNumberDB;

    // database ID
    uint32_t m_nDBId;

public:
    explicit KevDemoDatabase();
    virtual ~KevDemoDatabase();

    KevDemoError_t open(uint32_t nDBId);
    void close();

    /**
     * @brief This function is used to register authentication information including EV number and user photo.
     * @param nId authentication information ID
     * @param rEVNumber electical vehicle number
     * @param rUserPhoto user photo
     */
    inline void registerAuthInfo(uint32_t nId, QString rEVNumber, QImage rUserPhoto) {
        if(m_rEVNumberDB.count(nId) != 0 || m_rUserPhotoDB.count(nId) != 0) {
            return;
        }

        // register EV number and user photo at the given index's position
        m_rUserPhotoDB[nId] = rUserPhoto;
        m_rEVNumberDB[nId] = rEVNumber;
    }

    /**
     * @brief This function returns the number of an electical vehicle which is indicated by the given ID.
     * @param nId electical vehicle ID
     * @return vehicle number
     */
    inline QString getEVNumber(uint32_t nId) {
        if(m_rEVNumberDB.count(nId)) {
            return m_rEVNumberDB[nId];
        }
        return QString("NOT FOUND");
    }

    /**
     * @brief This function returns an user photo indicated by the given ID.
     * @param nId user index
     * @return user photo
     */
    inline QImage getUserPhoto(uint32_t nId) {
        if(m_rUserPhotoDB.count(nId)) {
            return m_rUserPhotoDB[nId];
        }
        return QImage();
    }
};

#endif // _KEV_DEMO_DATABASE_H_
