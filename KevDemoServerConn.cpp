#include "KevDemoServerConn.h"

KevDemoServerConn::KevDemoServerConn()
{
}

KevDemoServerConn::~KevDemoServerConn()
{
}

/**
 * @brief This function initialize the server and set the given IP.
 * @param rServerIP a scheduling server IP
 * @return error information
 */
KevDemoError_t
KevDemoServerConn::open(QString rServerIP)
{
    return KEV_SUCCESS;
}

/**
 * @brief This function is used to connect ot the given charger IP.
 * @param rChargerIP an electric vehicle charger IP
 * @return error information
 */
KevDemoError_t
KevDemoServerConn::connect(QString rChargerIP)
{
    return KEV_SUCCESS;
}

/**
 * @brief This function reads the bytes as many as nRead, received through a server connection,
 *        and return using rReadBuffer.
 * @param rReadBuffer a read buffer
 * @param nRead the number of bytes to be read
 * @return the actual number of read bytes
 */
int
KevDemoServerConn::read(uint8_t rReadBuffer[], int nRead)
{
    return 0;
}

/**
 * @brief This function writes the {nWrite} bytes of the given write buffer through a server connection.
 * @param rWriteBuffer a write buffer
 * @param nWrite the number of bytes to be written
 * @return the actual number of written bytes
 */
int
KevDemoServerConn::write(uint8_t rWriteBuffer[], int nWrite)
{
    return 0;
}

/**
 * @brief This function is used to close the server connection.
 */
void
KevDemoServerConn::close()
{

}
