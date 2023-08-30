#include "KevDemoVBCReader.h"

#ifdef KEV_VBC_SERIAL_ENABLE
#include <wiringSerial.h>
#endif

/**
 * @brief a constructor of VBC reader
 */
KevDemoVBCReader::KevDemoVBCReader()
{
#ifdef KEV_VBC_SERIAL_ENABLE
    m_nSerialCom = 0;
#endif
    m_pThread = NULL;
    m_bIsRunning = false;
    m_pVBCDecoder = NULL;
}

/**
 * @brief a destructor of VBC reader
 */
KevDemoVBCReader::~KevDemoVBCReader()
{
    if(m_pVBCDecoder != NULL)
    {
        delete m_pVBCDecoder;
    }
}

//////////////////////////////////////////////////
// Member Function Definition
//////////////////////////////////////////////////

/**
 * @brief This function is used to establish the VBC communication using serial interface.
 * @param nBaudrate baudrate of serial communication to read vibration signals
 * @param nThreshold threshold of serial communication
 * @param nPeriod the period of a VBC symbol
 * @return error information
 */
KevDemoError_t
KevDemoVBCReader::open(uint32_t nBaudrate, uint32_t nThreshold, uint32_t nPeriod)
{
    KevDemoError_t error;
    m_nBaudrate = nBaudrate;

#ifdef KEV_VBC_SERIAL_ENABLE
    // serial communication initialization
    if((m_nSerialCom = serialOpen("/dev/ttyAMA0", nBaudrate)) < 0)
    {
        emit sig_printDebugMessage(QString("Serial communication is not opened."));
        return KEV_ERROR_VBC_NOT_OPENED;
    }
#else
    if((error = loadSignalFile("/home/sun/Temp/temp.txt")) != KEV_SUCCESS)
    {
        emit sig_printDebugMessage(QString("Serial temp file is not loaded."));
        return error;
    }
#endif

    // initialize data queue and decoder for VBC
    m_rVBCDataQueue.clear();

    if(m_pVBCDecoder != NULL)
    {
        delete m_pVBCDecoder;
    }

    // VBC decoder initialization (threshold, period)
    m_pVBCDecoder = new KevDemoVBCDecoder(nThreshold, nPeriod);

    // Start the thread
    m_bIsRunning = true;
    this->start();

    return KEV_SUCCESS;
}

/**
 * @brief This function is used to close the VBC reader.
 */
void
KevDemoVBCReader::close()
{
#ifdef KEV_VBC_SERIAL_ENABLE
    serialClose(m_nSerialCom);
#endif
    m_bIsRunning = false;
    m_rVBCDataQueue.clear();

    if(m_pVBCDecoder != NULL)
    {
        delete m_pVBCDecoder;
    }
}

//////////////////////////////////////////////////
// Member Function Definition
//////////////////////////////////////////////////

/**
 * @brief This function is used to read a decoded byte over VBC.
 * @return a decoded byte
 */
int
KevDemoVBCReader::readByte()
{
    if(m_rVBCDataQueue.empty())
    {
       return -1;
    }

    return m_rVBCDataQueue.dequeue();
}

/**
 * @brief This function is used to read decoded bytes over VBC.
 * @param rReadBytes decoded read bytes
 * @param nNumBytes the number of bytes to read
 * @return the number of actually read bytes
 */
int
KevDemoVBCReader::readBytes(std::vector<int> &rReadBytes, int nNumBytes)
{
    int qsize = m_rVBCDataQueue.size();

    if(nNumBytes > qsize)
    {
        nNumBytes = qsize;
    }

    rReadBytes.clear();

    for(int i = 0; i < nNumBytes; i++)
    {
        rReadBytes.push_back(m_rVBCDataQueue.dequeue());
    }

    return nNumBytes;
}

/**
 * @brief This function is a slot function to be called when a sample is obtained.
 */
void
KevDemoVBCReader::run()
{
    char readBuf[16];
    uint32_t readValue;
    uint32_t decodedByte = 0;

    for(int i = 0, nbits = 0; m_bIsRunning == true; )
    {
#ifdef KEV_VBC_SERIAL_ENABLE
        if(!serialDataAvail(m_nSerialCom))
        {
            continue;
        }

        int readByte = serialGetchar(m_nSerialCom);

        if(readByte == '\n' || i == 5)
        {
            readBuf[i] = 0;

            // convert a string to an integer number
            readValue = QString(readBuf).toUInt();
            readValue &= KEV_VBC_MASK_DW16;

            // push the read value into a signal queue
            int decodedBit = m_pVBCDecoder->decode(readValue);

            if(decodedBit >= 0)
            {
                QString str("Decoded bit: ");
                str.append(QString::number(decodedBit));
                emit sig_printDebugMessage(str);

                // accumulate decoded bits into a decoded byte
                decodedByte = (decodedByte << 1) | decodedBit;

                nbits++;
            }

            i = 0;
        }
        else
        {
            readBuf[i++] = readByte;
        }
#else
        if(m_rVBCSignalBuffer.empty())
        {
            m_bIsRunning = false;
            continue;
        }

        readValue = m_rVBCSignalBuffer.dequeue();

        // push the read value into a signal queue
        int decodedBit = m_pVBCDecoder->decode(readValue);

        if(decodedBit >= 0)
        {
#if 0
            QString str("Decoded bit: ");
            str.append(QString::number(decodedBit));
            emit sig_printDebugMessage(str);
#endif
            // accumulate decoded bits into a decoded byte
            decodedByte = (decodedByte << 1) | decodedBit;

            nbits++;
        }
#endif
        // enqueue a data into VBC data queue
        if(nbits >= KEV_VBC_NUM_BITS_PER_BYTE)
        {
            m_rVBCDataQueue.push_back(decodedByte);
            nbits = 0;
#if 1
            QString str("> Decoded bits: ");
            str.append(QString::number(decodedByte, 16));
            emit sig_printDebugMessage(str);
#endif
            // finish to authenticate a vehicle number using VBC in this version of implementation.
            emit sig_performAuthentication(decodedByte);
            m_bIsRunning = false;
        }
    }
}

#ifndef KEV_VBC_SERIAL_ENABLE
/**
 * @brief This function loads a signal file at the given path and enqueues into the buffer.
 * @param rPath a signal file path
 * @return error information
 */
KevDemoError_t
KevDemoVBCReader::loadSignalFile(QString rPath)
{
    FILE *fp;
    char readBuf[16];
    uint32_t readValue;

    if((fp = fopen(rPath.toStdString().c_str(), "r")) == NULL)
    {
        return KEV_ERROR_VBC_NOT_OPENED;
    }

    m_rVBCSignalBuffer.clear();

    while(fscanf(fp, "%s\n", readBuf) != EOF)
    {
        // convert a string to an integer number
        readValue = QString(readBuf).toUInt();
        readValue &= KEV_VBC_MASK_DW16;

        // enqueue
        m_rVBCSignalBuffer.push_back(readValue);
    }

    fclose(fp);

    return KEV_SUCCESS;
}
#endif
