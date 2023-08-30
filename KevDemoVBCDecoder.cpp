#include "KevDemoVBCDecoder.h"
#include <algorithm>

/**
 * @brief Constructor of VBC decoder
 * @param nThreshold signal threshold for VLC
 * @param nPeriod signal period for VLC
 */
KevDemoVBCDecoder::KevDemoVBCDecoder(uint32_t nThreshold, uint32_t nPeriod)
{
    m_nSymbolCount = 0;
    m_nSampleCount = 0;
    m_rSampleQueue.clear();
    m_rSampleWindow.clear();
    m_nThreshold = nThreshold;
    m_nPeriod = nPeriod;
    m_nPeakValue = 0;
    m_nVBCState = KEV_VBC_STATE_IDLE;
}

/**
 * @brief Destructor of VBC decoder
 */
KevDemoVBCDecoder::~KevDemoVBCDecoder()
{
    m_rSampleQueue.clear();
    m_rSampleWindow.clear();
}

/**
 * @brief This function decodes the input values that are received through vibration communication.
 * @param nReadValue input signal value
 * @return 0 or 1 if there is a decoded bit, otherwise returns -1.
 */
int
KevDemoVBCDecoder::decode(uint32_t nReadValue)
{
    int readValue = nReadValue - m_nThreshold;

    if(readValue < 0)
    {
        readValue = 0;
    }

    m_rSampleWindow.push_back(readValue);

    if(m_rSampleWindow.size() > KEV_VBC_WINDOW_SIZE)
    {
        m_rSampleWindow.pop_front();
    }

    // get max value from VBC window
    uint32_t maxValue = *std::max_element(m_rSampleWindow.constBegin(),
                                          m_rSampleWindow.constEnd());
    // state transition
    switch(m_nVBCState)
    {
        case KEV_VBC_STATE_IDLE:
        {
            if(maxValue > 0)
            {
                m_nVBCState = KEV_VBC_STATE_SYNC;
                m_rSampleQueue.clear();
                m_nSampleCount = 0;
                m_nSymbolCount = 0;
                m_nPeakValue = 0;
            }
            break;
        }
        case KEV_VBC_STATE_SYNC:
        {
            m_nSampleCount++;
            m_rSampleQueue.push_back(maxValue);

            if(m_nSymbolCount == 0)
            {
                // find a peak value
                if(maxValue > m_nPeakValue)
                {
                    m_nPeakValue = maxValue;
                }
                // eliminate transient vibration signals at the beginning
                else if(maxValue <= 0 && m_nSampleCount < m_nPeriod)
                {
                    m_nSampleCount = 0;
                    m_nPeakValue = 0;
                }
            }
            else if(m_nSymbolCount == 2)
            {
                uint32_t halfPeak = m_nPeakValue/2;

                // find a period of a vibration signal
                if(maxValue <= halfPeak && m_nSampleCount < (m_nPeriod/2))
                {
                    m_nSampleCount = 0;
                }
            }

            // count a new symbol
            if(m_nSampleCount > m_nPeriod)
            {
                m_nSampleCount = 0;
                m_nSymbolCount++;
            }

            // start data transition
            if(m_nSymbolCount >= KEV_VBC_NUM_SYNC_SYMBOLS)
            {
                m_nVBCState = KEV_VBC_STATE_DATA;
                m_nSampleCount = 0;
                m_nSymbolCount = 0;
                m_rSampleQueue.clear();
            }
            break;
        }
        case KEV_VBC_STATE_DATA:
        {
            m_rSampleQueue.push_back(maxValue);

            if(m_rSampleQueue.size() > (int)m_nPeriod)
            {
                //uint32_t midValue = m_rSampleQueue.at((int)m_nPeriod/2);
                uint32_t midValue = getAverageSamples();

                // clear the sample queue for the current symbols
                m_rSampleQueue.clear();

                if(++m_nSymbolCount >= KEV_VBC_NUM_DATA_SYMBOLS)
                {
                    m_nVBCState = KEV_VBC_STATE_IDLE;
                    m_nSampleCount = 0;
                }

                // determine whether the current symbol is 1 or 0
                if(midValue > (m_nPeakValue/2))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }

            break;
        }
        default:
            return KEV_ERROR_UNKNOWN_VBC_STATE;
    }

    return -1;
}

/**
 * @brief This function returns the average of enqueued samples.
 * @return average value of enqueued samples
 */
uint32_t
KevDemoVBCDecoder::getAverageSamples()
{
    uint32_t sum = 0;

    for(auto sample : m_rSampleQueue)
    {
        sum += sample;
    }

    return sum/m_rSampleQueue.size();
}
