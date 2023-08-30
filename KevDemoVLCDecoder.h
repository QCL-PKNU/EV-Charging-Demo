#ifndef _KEV_DEMO_VLC_DECODER_H_
#define _KEV_DEMO_VLC_DECODER_H_

#include "KevDemoConfig.h"
#include "KevDemoROIBlock.h"

// VLC decoders
#define KEV_VLC_DEC_MI  0
#define KEV_VLC_DEC_RS  1

// VLC states
#define KEV_VLC_STATE_IDLE  0
#define KEV_VLC_STATE_SYNC  1
#define KEV_VLC_STATE_DATA  2

// VLC default number of frames
#define KEV_VLC_NUM_IDLE_FRAMES  10
#define KEV_VLC_NUM_SYNC_FRAMES  20
#define KEV_VLC_NUM_DATA_FRAMES  60

// VLC default threshold
#define KEV_VLC_DEFAULT_THRESHOLD   128

#define KEV_VLC_MANCH_HOLDING     0
#define KEV_VLC_MANCH_FALLING     1
#define KEV_VLC_MANCH_RISING      2

/**
 * @brief a class for VLC decoding
 */
class KevDemoVLCDecoder : public QObject
{
    Q_OBJECT

private:

    const cv::Scalar COLOR_BLOBS = cv::Scalar( 10,  10,  10);
    const cv::Scalar COLOR_BLACK = cv::Scalar(  0,   0,   0);
    const cv::Scalar COLOR_WHITE = cv::Scalar(255, 255, 255);

    // threshold to obtain a B&W difference image
    uint32_t m_nThreshold;

    // decoder type
    uint32_t m_nDecodeType;
    // decoder data width
    uint32_t m_nDataWidth;

    // previous frame
    cv::Mat m_rPrevFrame;
    // sync frame
    cv::Mat m_rSyncFrame;
    // background frame
    cv::Mat m_rBgrdFrame;

    // VLC state
    uint32_t m_nVLCState;

    // means for ROI blocks
    std::vector<float> m_rMeanROIs;

    // detected ROI blocks for VLC
    std::vector<KevDemoROIBlock> m_rDetectedROIs;

    // number of consecutive invalid frames
    uint32_t m_nNumConsEmptyFrames;

    // frame counter
    uint32_t m_nFrameCounter;

    // VLC clock index
    int m_nClockIndex;

    // previous decoded bits
    int m_rPrevClock;

    /**
     * @brief an internal class for presenting Blobs
     */
    class VLCBlob
    {
    public:
        double aspectRatio;
        double diagonalSize;

        cv::Rect boundingRect;
        cv::Point centerPosition;

        std::vector<cv::Point> contour;

        /**
         * @brief a constructor of VLC blob
         * @param a contour used to build a VLC blob
         */
        VLCBlob(std::vector<cv::Point> rContour)
        {
            contour = rContour;
            boundingRect = cv::boundingRect(rContour);

            centerPosition.x = (boundingRect.x + boundingRect.x + boundingRect.width) / 2;
            centerPosition.y = (boundingRect.y + boundingRect.y + boundingRect.height) / 2;

            diagonalSize = sqrt(pow(boundingRect.width, 2) + pow(boundingRect.height, 2));
            aspectRatio = (float)boundingRect.width / (float)boundingRect.height;
        }

        /**
         * @brief This function is used to validate the ROI block.
         * @return true if the block is a valid ROI, otherwise false
         */
        inline bool isValid()
        {
            if(boundingRect.area() < 60 || boundingRect.area() > 2000)  return false;
            if(aspectRatio < 0.2 || aspectRatio > 1.25)                 return false;
            if(boundingRect.width < 15 || boundingRect.height < 15)     return false;
            if(diagonalSize < 20)                                       return false;
            return true;
        }
    };

signals:

    void sig_performAuthentication(int nAuthInfo);

    void sig_printDebugMessage(QString rString);

public:

    explicit KevDemoVLCDecoder(int nDecodeType = KEV_VLC_DEC_MI);
    virtual ~KevDemoVLCDecoder();

    // VLC decoding procedures
    KevDemoError_t decode(cv::Mat& rCurrFrame, std::vector<int>& rDecodedOutput);

    // accessor & mutator
    inline void setThreshold(uint32_t nThreshold)   { m_nThreshold = nThreshold; }
    inline uint32_t getThreshold()                  { return m_nThreshold;       }

    inline void setDataWidth(uint32_t nDataWidth)   { m_nDataWidth = nDataWidth; }
    inline uint32_t getDataWidth()                  { return m_nDataWidth;       }

    inline void setClockIndex(uint32_t nClockIndex) { m_nClockIndex = nClockIndex; }
    inline uint32_t getClockIndex()                 { return m_nClockIndex;        }

private:

    /**
     * @brief This function checks whether the VLC enters into IDLE state or not.
     * @return true if the VLC is in IDLE state, otherwise false.
     */
    inline bool isIdleDetected()
    {
        return m_nNumConsEmptyFrames > KEV_VLC_NUM_IDLE_FRAMES;
    }

    // detect sync start frame
    bool detectSyncStart(cv::Mat rFrame);

    // detect ROIs
    bool detectROIs(cv::Mat rSyncFrame, std::vector<KevDemoROIBlock>& rROIBlocks);

    // decode a sync frame
    KevDemoError_t decodeSyncFrame(cv::Mat rCurrFrame, std::vector<VLCBlob>& rBlobs);
    KevDemoError_t decodeSyncMIFrame(cv::Mat rCurrFrame, std::vector<VLCBlob>& rBlobs);
    KevDemoError_t decodeSyncRSFrame(cv::Mat rCurrFrame, std::vector<VLCBlob>& rBlobs);

    // decode a data frame using detected ROIs
    KevDemoError_t decodeDataFrame(cv::Mat rDataFrame, std::vector<int>& rDecodedSignals);
    KevDemoError_t decodeDataMIFrame(cv::Mat rDataFrame, std::vector<int>& rDecodedSignals);
    KevDemoError_t decodeDataRSFrame(cv::Mat rDataFrame, std::vector<int>& rDecodedSignals);

    // internal procedures for decoding
    KevDemoError_t subtractFrame(cv::Mat rPrevFrame, cv::Mat rCurrFrame, cv::Mat& rSubFrame);
    KevDemoError_t obtainDiffFrame(cv::Mat rPrevFrame, cv::Mat rCurrFrame, cv::Mat& rDiffFrame);
    KevDemoError_t filterMorphology(cv::Mat& rFrame, int nFilterSize);
    KevDemoError_t detectBlobs(cv::Mat rFrame, std::vector<VLCBlob>& rBlobs);

    // draw a blob frame
    void drawBlobsToFrame(cv::Mat &rFrame, std::vector<VLCBlob> rBlobs);
};

#endif // _KEV_DEMO_VLC_DECODER_H_
