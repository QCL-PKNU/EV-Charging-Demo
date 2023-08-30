#ifndef _KEV_DEMO_VBC_READER_H_
#define _KEV_DEMO_VBC_READER_H_

#include "KevDemoConfig.h"
#include "KevDemoVBCDecoder.h"

#define KEV_VBC_MASK_DW08   0x000000FF
#define KEV_VBC_MASK_DW16   0x0000FFFF
#define KEV_VBC_MASK_DW32   0xFFFFFFFF

#define KEV_VBC_NUM_BITS_PER_BYTE   8

/**
 * @brief a class for reading VBC signals
 */
class KevDemoVBCReader : public QThread
{
    Q_OBJECT

private:

    // VBC queue
    QQueue<uint32_t> m_rVBCDataQueue;

    // VBC decoder
    KevDemoVBCDecoder *m_pVBCDecoder;

    // baudrate of serial communication to read vibration signals
    uint32_t m_nBaudrate;

#ifdef KEV_VBC_SERIAL_ENABLE
    // a file descriptor for serial communication
    int m_nSerialCom;
#else
    // a buffer to load VBC signals from a signal file
    QQueue<uint32_t> m_rVBCSignalBuffer;
#endif

    // VBC reader thread
    QThread *m_pThread;

    // VBC reader flag
    bool m_bIsRunning;

public:

    explicit KevDemoVBCReader();
    virtual ~KevDemoVBCReader();

    /**
     * @brief This function returns whether the reader is now running or not.
     * @return true if the reader is operating, otherwise false.
     */
    inline bool isRunning() { return m_bIsRunning; }

    // open & close the reader
    KevDemoError_t open(uint32_t nBaudrate, uint32_t nThreshold, uint32_t nPeriod);
    void close();

    // read operations
    int readByte();
    int readBytes(std::vector<int> &rReadBytes, int nNumBytes);

    // thread
    void run();

signals:

    void sig_performAuthentication(int nAuthInfo);

    void sig_printDebugMessage(QString rString);

private:

#ifndef KEV_VBC_SERIAL_ENABLE
    // load a temporary file
    KevDemoError_t loadSignalFile(QString rPath);
#endif
};

#endif // _KEV_DEMO_VBC_READER_H_
