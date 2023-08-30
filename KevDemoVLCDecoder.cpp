#include "KevDemoVLCDecoder.h"

//////////////////////////////////////////////////
// Constructor & Destructor Definition
//////////////////////////////////////////////////

/**
 * @brief This is a constructor of KevDemoVLCDecoder class
 * @param nDecodeType decoder type
 */
KevDemoVLCDecoder::KevDemoVLCDecoder(int nDecodeType)
{
    m_rPrevFrame = cv::Mat();
    m_nDecodeType = nDecodeType;
    m_nVLCState = KEV_VLC_STATE_IDLE;
    m_nThreshold = KEV_VLC_DEFAULT_THRESHOLD;
    m_rDetectedROIs.clear();
    m_nNumConsEmptyFrames = 0;
    m_nFrameCounter = 0;
    m_nDataWidth = 0;
    m_nClockIndex = 0;
}

/**
 * @brief This is a destructor of KevDemoVLCDecoder class
 */
KevDemoVLCDecoder::~KevDemoVLCDecoder()
{
    m_rPrevFrame.release();
}

//////////////////////////////////////////////////
// Member Function Definition
//////////////////////////////////////////////////

/**
 * @brief This function is used to decode a frame using a specifc type of decoder.
 * @param rCurrFrame the current frame to be decoded
 * @param rDecodedBits decoded output bits
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::decode(cv::Mat& rCurrFrame, std::vector<int>& rDecodedBits)
{
    if(m_rPrevFrame.empty() == true)
    {
        m_rPrevFrame = rCurrFrame.clone();
        return KEV_SUCCESS;
    }

    // IDLE state check
    if(isIdleDetected())
    {
        m_nVLCState = KEV_VLC_STATE_IDLE;
        m_nNumConsEmptyFrames = 0;
        m_nFrameCounter = 0;
    }

    // VLC state machine
    switch(m_nVLCState)
    {
        case KEV_VLC_STATE_IDLE:
        {
            m_rDetectedROIs.clear();

            // try to detect a sync start frame
            if(detectSyncStart(rCurrFrame) == true)
            {
                m_nVLCState = KEV_VLC_STATE_SYNC;
                m_nFrameCounter = 0;
            }
            break;
        }
        case KEV_VLC_STATE_SYNC:
        {
            // detects ROI blocks for VLC
            if(detectROIs(rCurrFrame, m_rDetectedROIs) == true)
            {
                m_nVLCState = KEV_VLC_STATE_DATA;
                m_nFrameCounter = 0;
                m_rPrevClock = 0;
            }
            break;
        }
        case KEV_VLC_STATE_DATA:
        {
            std::vector<int> decodedSignals;

            // decode a data frame
            decodeDataFrame(rCurrFrame, decodedSignals);

            if(++m_nFrameCounter >= KEV_VLC_NUM_DATA_FRAMES)
            {
                m_nVLCState = KEV_VLC_STATE_IDLE;
                m_nFrameCounter = 0;
            }
            else
            {
#if 1
                // return decoded bits at clock rising edge
                int currClock = decodedSignals[m_nClockIndex];

                if(currClock != m_rPrevClock && currClock == 1)
                {
                    for(uint32_t i = 0; i < decodedSignals.size(); i++)
                    {
                        if(i == m_nClockIndex)
                        {
                            continue;
                        }

                        rDecodedBits.push_back(decodedSignals[i]);
                    }
                }

                m_rPrevClock = currClock;
#else
                for(uint32_t i = 0; i < decodedSignals.size(); i++)
                {
                    rDecodedBits.push_back(decodedSignals[i]);
                }
#endif
            }
            break;
        }
        default:
            return KEV_ERROR_UNKNOWN_VLC_STATE;
    }

    // update the previous frame
    m_rPrevFrame = rCurrFrame.clone();

    // draw a red rectangle over the current frmae for each blob
    for(KevDemoROIBlock &block : m_rDetectedROIs)
    {
        cv::rectangle(rCurrFrame, block.getBoundingRect(), COLOR_WHITE, 1);
    }

    return KEV_SUCCESS;
}

/**
 * @brief This function is used to decode a frame using a specifc type of decoder.
 * @param rCurrFrame the current frame to be decoded
 * @param rBlobs a list of detected blobs
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::decodeSyncFrame(cv::Mat rCurrFrame, std::vector<VLCBlob>& rBlobs)
{
    KevDemoError_t error;

    if(m_nDecodeType == KEV_VLC_DEC_MI)
    {
        error = decodeSyncMIFrame(rCurrFrame, rBlobs);
    }
    else if(m_nDecodeType == KEV_VLC_DEC_RS)
    {
        error = decodeSyncRSFrame(rCurrFrame, rBlobs);
    }
    else
    {
        error = KEV_ERROR_UNKNOWN_VLC_DECODER;
    }

    return error;
}

/**
 * @brief This function decodes a sync frame using MIMO decoder.
 * @param rCurrFrame the current frame to be decoded
 * @param rBlobs a list of detected ROI blocks
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::decodeSyncMIFrame(cv::Mat rCurrFrame, std::vector<VLCBlob>& rBlobs)
{
    KevDemoError_t error = KEV_SUCCESS;

    cv::Mat diffFrame;

    // obtain the difference of the current and previous frames
    if((error = obtainDiffFrame(m_rPrevFrame, rCurrFrame, diffFrame)) != KEV_SUCCESS)
    {
        return error;
    }

    // perform morphology filtering
    if((error = filterMorphology(diffFrame, 5)) != KEV_SUCCESS)
    {
        return error;
    }

    // detect blobs of the difference frame
    if((error = detectBlobs(diffFrame, rBlobs)) != KEV_SUCCESS)
    {
        return error;
    }

    return error;
}

/**
 * @brief This function decodes the current frame using Rolling Shutter decoder.
 * @param rCurrFrame the current frame to be decoded
 * @param rBlobs a list of detected ROI blocks
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::decodeSyncRSFrame(cv::Mat rCurrFrame, std::vector<VLCBlob>& rBlobs)
{
    KevDemoError_t error = KEV_SUCCESS;

    // Rolling Shutter Algorithm

    return error;
}


/**
 * @brief This function is used to subtract the previous frame with the current frame.
 * @param rPrevFrame a previous frame
 * @param rCurrFrame a current frame
 * @param rDiffFrame a result substract frame
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::subtractFrame(cv::Mat rPrevFrame, cv::Mat rCurrFrame, cv::Mat &rSubFrame)
{
    if(rPrevFrame.empty() == true || rCurrFrame.empty() == true)
    {
        return KEV_ERROR_INVALID_ARGUMENTS;
    }

    cv::GaussianBlur(rPrevFrame, rPrevFrame, cv::Size(5, 5), 0);
    cv::GaussianBlur(rCurrFrame, rCurrFrame, cv::Size(5, 5), 0);

    // obtain a substraction image
    cv::subtract(rPrevFrame, rCurrFrame, rSubFrame);

    // thresholding the substract image into a black & white image
    cv::threshold(rSubFrame, rSubFrame, m_nThreshold, 255, CV_THRESH_BINARY);

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
    cv::imshow("subtractFrame", rSubFrame);
#endif
    return KEV_SUCCESS;
}

/**
 * @brief This function is used to obtain the difference of the current frame from its previous frame.
 * @param rPrevFrame a previous frame
 * @param rCurrFrame a current frame
 * @param rDiffFrame a result difference frame
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::obtainDiffFrame(cv::Mat rPrevFrame, cv::Mat rCurrFrame, cv::Mat &rDiffFrame)
{
    if(rPrevFrame.empty() == true || rCurrFrame.empty() == true)
    {
        return KEV_ERROR_INVALID_ARGUMENTS;
    }

    cv::GaussianBlur(rPrevFrame, rPrevFrame, cv::Size(5, 5), 0);
    cv::GaussianBlur(rCurrFrame, rCurrFrame, cv::Size(5, 5), 0);

    // obtain a difference image
    cv::absdiff(rPrevFrame, rCurrFrame, rDiffFrame);

    // thresholding the difference image into a black & white image
    cv::threshold(rDiffFrame, rDiffFrame, m_nThreshold, 255, CV_THRESH_BINARY);

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
    cv::imshow("obtainDiffFrame", rDiffFrame);
#endif
    return KEV_SUCCESS;
}

/**
 * @brief This function performs morphological filtering with the given size of filters.
 * @param rFrame a frame to be filtered
 * @param nFilterSize filter size
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::filterMorphology(cv::Mat &rFrame, int nFilterSize)
{
    if(rFrame.empty() == true || nFilterSize <= 0)
    {
        return KEV_ERROR_INVALID_ARGUMENTS;
    }

    cv::Size filterSize(nFilterSize, nFilterSize);
    cv::Mat filterElement = cv::getStructuringElement(cv::MORPH_RECT, filterSize);

    cv::dilate(rFrame, rFrame, filterElement);
    cv::dilate(rFrame, rFrame, filterElement);
    cv::erode( rFrame, rFrame, filterElement);

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
    cv::imshow("filterMorphology", rFrame);
#endif
    return KEV_SUCCESS;
}

/**
 * @brief This function detects blobs of the given frame.
 * @param rFrame current image frame
 * @param rBlobs a list of detected blobs
 * @return
 */
KevDemoError_t
KevDemoVLCDecoder::detectBlobs(cv::Mat rFrame, std::vector<VLCBlob> &rBlobs)
{
    if(rFrame.empty() == true)
    {
        return KEV_ERROR_INVALID_ARGUMENTS;
    }

    // clear a list of blobs
    rBlobs.clear();

    // find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(rFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // find convex hulls
    std::vector<std::vector<cv::Point> > convexHulls(contours.size());

    for(uint32_t i = 0; i < contours.size(); i++)
    {
        cv::convexHull(contours[i], convexHulls[i]);
    }

    // add ROI blocks to the given block list
    for(auto &convexHull : convexHulls)
    {
        VLCBlob blob(convexHull);

        if(blob.isValid() == true)
        {
            rBlobs.push_back(blob);
        }
    }

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
    cv::Mat imgConvexHulls(rFrame.size(), CV_8UC3, COLOR_BLACK);
    // to draw only the valid convex hulls
    convexHulls.clear();
    for (auto &blob : rBlobs) {
        convexHulls.push_back(blob.contour);
    }
    cv::drawContours(imgConvexHulls, convexHulls, -1, COLOR_WHITE, -1);
    cv::imshow("imgConvexHulls", imgConvexHulls);
#endif
    return KEV_SUCCESS;
}

/**
 * @brief This function detects VLC sync start frame.
 * @param rFrame an image frame
 * @return true if the given frame is a sync start frame, otherwise false;
 */
bool
KevDemoVLCDecoder::detectSyncStart(cv::Mat rFrame)
{
    std::vector<VLCBlob> blobs;

    // find blobs from the current frame
    KevDemoError_t error = decodeSyncFrame(rFrame, blobs);

    if(error != KEV_SUCCESS || blobs.size() != m_nDataWidth)
    {
        m_nNumConsEmptyFrames++;
        return false;
    }
    else
    {
        m_nNumConsEmptyFrames = 0;
    }

    // create a background frame
    m_rBgrdFrame = cv::Mat::zeros(rFrame.size(), CV_32FC1);

    // create a sync frame by drawing blobs on a black frame
    m_rSyncFrame = cv::Mat::zeros(rFrame.size(), CV_32FC3);
    drawBlobsToFrame(m_rSyncFrame, blobs);

    emit sig_printDebugMessage(QString("Sync Start..."));

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
    cv::imshow("detectSyncStart", m_rSyncFrame);
#endif
    return true;
}

/**
 * @brief This function detects ROI (Region-of-Interest) blocks from sync frames.
 * @param rSyncFrame a sync frame
 * @param rROIBlocks detected ROI blocks
 * @return true if detection of ROI blocks completes, otherwise false
 */
bool
KevDemoVLCDecoder::detectROIs(cv::Mat rSyncFrame, std::vector<KevDemoROIBlock> &rROIBlocks)
{
    if(rSyncFrame.empty() == true || m_rSyncFrame.empty() == true)
    {
        return false;
    }

    // decode a sync frame
    std::vector<VLCBlob> blobs;
    KevDemoError_t error = decodeSyncFrame(rSyncFrame, blobs);

    if(error != KEV_SUCCESS)
    {
        m_nNumConsEmptyFrames++;
        return false;
    }
    else
    {
        m_nNumConsEmptyFrames = 0;
    }

    cv::Mat syncFrame(rSyncFrame.size(), CV_8UC3, COLOR_BLACK);
    drawBlobsToFrame(syncFrame, blobs);

    // accumulate sync frames to build a background frame
    cv::accumulate(rSyncFrame, m_rBgrdFrame);

    // accumulate sync frames to detect ROIs
    cv::accumulate(syncFrame, m_rSyncFrame);

    // if all the sync frames are received, find ROI blocks and change the state into KEV_DEMO_STATE_DATA.
    if(++m_nFrameCounter >= KEV_VLC_NUM_SYNC_FRAMES)
    {
        cv::Mat roiFrame;
        cv::cvtColor(m_rSyncFrame, syncFrame, CV_RGB2GRAY);
        syncFrame.convertTo(roiFrame, CV_8UC1);

        // detect blobs of the accumulated sync frame
        error = detectBlobs(roiFrame, blobs);

        // return false if the number of detected blobs of a sync frame is different from the data width
        if(error != KEV_SUCCESS || blobs.size() != m_nDataWidth)
        {
            emit sig_printDebugMessage(QString("ROI Detection Error...Retry!"));
            m_nVLCState = KEV_VLC_STATE_IDLE;
            return false;
        }

        // obtain a background image
        m_rBgrdFrame /= KEV_VLC_NUM_SYNC_FRAMES;
        m_rMeanROIs.clear();

        // build a list of ROI blocks using blobs
        rROIBlocks.clear();        

        for(VLCBlob &blob : blobs)
        {
            rROIBlocks.push_back(KevDemoROIBlock(blob.contour));

            // calculate means of ROI images
            cv::Mat roiImage = cv::Mat(m_rBgrdFrame, blob.boundingRect);
            m_rMeanROIs.push_back(cv::mean(roiImage).val[0]);
        }

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
        cv::imshow("detectROIs", roiFrame);
#endif
        emit sig_printDebugMessage(QString("ROI Detected."));
        return true;
    }

    return false;
}

/**
 * @brief This function decodes a data frame using detected ROIs and returns decoded bits.
 * @param rDataFrame input data frame
 * @param rDecodedSignals a list of decoded signals
 * @return
 */
KevDemoError_t
KevDemoVLCDecoder::decodeDataFrame(cv::Mat rDataFrame, std::vector<int> &rDecodedSignals)
{
    KevDemoError_t error;

    if(m_nDecodeType == KEV_VLC_DEC_MI)
    {
        error = decodeDataMIFrame(rDataFrame, rDecodedSignals);
    }
    else if(m_nDecodeType == KEV_VLC_DEC_RS)
    {
        error = decodeDataRSFrame(rDataFrame, rDecodedSignals);
    }
    else
    {
        error = KEV_ERROR_UNKNOWN_VLC_DECODER;
    }

    return error;
}

/**
 * @brief This function decodes a data frame using MIMO decoder.
 * @param rDataFrame a data frame to be decoded
 * @param rDecodedSignals a list of decoded signals
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::decodeDataMIFrame(cv::Mat rDataFrame, std::vector<int> &rDecodedSignals)
{
    int i = 0;
    rDecodedSignals.clear();

    for(KevDemoROIBlock &roiBlock : m_rDetectedROIs)
    {
        // a rectangle region of ROI
        cv::Rect roiRect = roiBlock.getBoundingRect();

        // extracted previous and current ROI images
        cv::Mat prevRoiImage = cv::Mat(m_rPrevFrame, roiRect);
        cv::Mat currRoiImage = cv::Mat(  rDataFrame, roiRect);

#if 1
        // obtain the decoded signal using the mean value of an ROI image
        float meanROI = cv::mean(currRoiImage).val[0];

        if(meanROI >= m_rMeanROIs[i])
        {
            rDecodedSignals.push_back(1);
        }
        else
        {
            rDecodedSignals.push_back(0);
        }
#else
        // obtain the difference of the previous and current ROI images
        cv::Mat subRoiImage1;
        cv::Mat subRoiImage2;

        subtractFrame(prevRoiImage, currRoiImage, subRoiImage1);
        subtractFrame(currRoiImage, prevRoiImage, subRoiImage2);

        // calculate the mean of ROI substraction images
        float meanPTC = cv::mean(subRoiImage1).val[0];
        float meanCTP = cv::mean(subRoiImage2).val[0];

        if(meanPTC < m_nThreshold) meanPTC = 0;
        if(meanCTP < m_nThreshold) meanCTP = 0;

        // determine VLC signals for this frame
        if(meanPTC > meanCTP)
        {
            rDecodedSignals.push_back(KEV_VLC_MANCH_FALLING);
        }
        else if(meanPTC < meanCTP)
        {
            rDecodedSignals.push_back(KEV_VLC_MANCH_RISING);
        }
        else
        {
            rDecodedSignals.push_back(KEV_VLC_MANCH_HOLDING);
        }
#endif

#ifdef KEV_VLC_DEC_DEBUG_ENABLE
        QString imgNumber1("ROI image1: ");
        imgNumber1 += QString::number(m_nFrameCounter);
        QString imgNumber2("ROI image2: ");
        imgNumber2 += QString::number(m_nFrameCounter);
        QString imgNumber3("ROI image3: ");
        imgNumber3 += QString::number(m_nFrameCounter);

        cv::imshow(imgNumber1.toStdString(), currRoiImage);
        cv::imshow(imgNumber2.toStdString(), subRoiImage1);
        cv::imshow(imgNumber3.toStdString(), subRoiImage2);

        QString str = QString("> P-C: ") + QString::number(meanPTC) +
                      QString(", C-P: ") + QString::number(meanCTP);
        emit sig_printDebugMessage(str);
#endif
    }

    return KEV_SUCCESS;
}

/**
 * @brief This function decodes a data frame using Rolling Shutter decoder.
 * @param rDataFrame a data frame to be decoded
 * @param rDecodedSignals a list of decoded signals
 * @return error information
 */
KevDemoError_t
KevDemoVLCDecoder::decodeDataRSFrame(cv::Mat rDataFrame, std::vector<int> &rDecodedSignals)
{
    KevDemoError_t error = KEV_SUCCESS;

    // Rolling Shutter Algorithm

    return error;
}

/**
 * @brief This function draws blobs on the given frame.
 * @param rFrame an image frame
 * @param rBlobs blobs to be drawn on the image frame
 */
void
KevDemoVLCDecoder::drawBlobsToFrame(cv::Mat &rFrame, std::vector<VLCBlob> rBlobs)
{
    // build a list of convexhulls
    std::vector<std::vector<cv::Point> > convexHulls;

    for (VLCBlob &blob : rBlobs) {
        convexHulls.push_back(blob.contour);
    }

    cv::drawContours(rFrame, convexHulls, -1, COLOR_BLOBS, -1);
}
