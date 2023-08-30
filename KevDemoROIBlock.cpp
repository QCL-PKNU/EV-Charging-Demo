#include "KevDemoROIBlock.h"

/**
 * @brief a constructor of ROI blocks
 * @param a contour used to build a ROI block
 */
KevDemoROIBlock::KevDemoROIBlock(std::vector<cv::Point> rContour)
{
    m_rBoundingRect = cv::boundingRect(rContour);
    m_rContour = rContour;
}

/**
 * @brief This function returns if a given ROI is overlapped with this ROI.
 * @param rBlock a given ROI
 * @return true if a given ROI is overlapped with this ROI, otherwise false.
 */
bool
KevDemoROIBlock::isOverlap(KevDemoROIBlock rBlock)
{
    if((m_rBoundingRect & rBlock.getBoundingRect()).area() > 0)
    {
        return true;
    }

    return false;
}
