#include "KevDemoDatabase.h"

KevDemoDatabase::KevDemoDatabase()
{
    m_rEVNumberDB.clear();
    m_rUserPhotoDB.clear();
}

KevDemoDatabase::~KevDemoDatabase()
{
    m_rEVNumberDB.clear();
    m_rUserPhotoDB.clear();
}

/**
 * @brief This function opens the database for storing authentication information.
 * @return error information
 */
KevDemoError_t
KevDemoDatabase::open(uint32_t nDBId) {

    m_nDBId = nDBId;

    // Reserved
    return KEV_SUCCESS;
}

/**
 * @brief This function closes the database that is being opened.
 */
void
KevDemoDatabase::close() {
    // Reserved
}
