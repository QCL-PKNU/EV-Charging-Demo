#ifndef _KEV_DEMO_SERVER_CONN_H_
#define _KEV_DEMO_SERVER_CONN_H_

#include "KevDemoConfig.h"

/**
 * @brief a class for supporting the server connection between ev chargers and a scheduling server.
 *        Currently, this class is not fully implemented and just reserved for the future usage.
 */
class KevDemoServerConn
{
private:

    // server IP
    QString m_rServerIP;

    // evcharger IP
    QString m_rChargerIP;

public:
    explicit KevDemoServerConn();
    virtual ~KevDemoServerConn();

    // member functions
    KevDemoError_t open(QString rServerIP);
    KevDemoError_t connect(QString rChargerIP);
    void close();

    // read & write functions
    int read(uint8_t rReadBuffer[], int nRead);
    int write(uint8_t rWriteBuffer[], int nWrite);
};

#endif // _KEV_DEMO_DATABASE_H_
