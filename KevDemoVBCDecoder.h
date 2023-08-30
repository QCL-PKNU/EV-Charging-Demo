#ifndef _KEV_DEMO_VBC_DECODER_H_
#define _KEV_DEMO_VBC_DECODER_H_

#include "KevDemoConfig.h"

// VBC states
#define KEV_VBC_STATE_IDLE  0
#define KEV_VBC_STATE_SYNC  1
#define KEV_VBC_STATE_DATA  2

// VBC default number of symbols
#define KEV_VBC_NUM_IDLE_SAMPLES  50
#define KEV_VBC_NUM_SYNC_SYMBOLS  4
#define KEV_VBC_NUM_DATA_SYMBOLS  8

// VBC window size
#define KEV_VBC_WINDOW_SIZE 9

/**
 * @brief a class for VBC decoding
 */
class KevDemoVBCDecoder : public QObject
{
    Q_OBJECT

private:

    // VBC signal queue
    QList<uint32_t> m_rSampleQueue;

    // VBC window to perform envelope detection
    QQueue<uint32_t> m_rSampleWindow;

    // sample counter
    uint32_t m_nSampleCount;

    // symbol counter
    uint32_t m_nSymbolCount;

    // threshold value
    uint32_t m_nThreshold;

    // signal period
    uint32_t m_nPeriod;

    // signal peak value
    uint32_t m_nPeakValue;

    // VBC state
    uint32_t m_nVBCState;

public:

    // constructor & destructor
    explicit KevDemoVBCDecoder(uint32_t nThreshold, uint32_t nPeriod);
    virtual ~KevDemoVBCDecoder();

    // member functions
    int decode(uint32_t nReadValue);

private:

    uint32_t getAverageSamples();
};

#endif // _KEV_DEMO_VBC_DECODER_H_
