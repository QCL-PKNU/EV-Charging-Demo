#ifndef _KEV_DEMO_ROI_BLOCK_H_
#define _KEV_DEMO_ROI_BLOCK_H_

#include "KevDemoConfig.h"

/**
 * @brief a class for presenting RoI (Region of Interest) block
 */
class KevDemoROIBlock
{
public:
    explicit KevDemoROIBlock(std::vector<cv::Point> rContour);

    // accessor
    inline cv::Rect getBoundingRect()
    {
        return m_rBoundingRect;
    }

    inline std::vector<cv::Point> getContour()
    {
        return m_rContour;
    }

    // member functions
    bool isOverlap(KevDemoROIBlock rBlock);

private:
    // bounding rectangule
    cv::Rect m_rBoundingRect;

    // contour
    std::vector<cv::Point> m_rContour;
};

#endif // _KEV_DEMO_ROI_BLOCK_H_
