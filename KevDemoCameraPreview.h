#ifndef _KEV_DEMO_CAMERA_PREVIEW_H_
#define _KEV_DEMO_CAMERA_PREVIEW_H_

#include "KevDemoConfig.h"

/**
 * @brief a class for previewing a camera frame by frame
 */
class KevDemoCameraPreview : public QObject
{
    Q_OBJECT

private:

    // camera preview
    cv::VideoCapture *m_pCameraPreview;

    // camera preview frame
    cv::Mat m_rPreviewFrame;

    // camera preview thread
    QThread *m_pThread;

public:

    explicit KevDemoCameraPreview();
    virtual ~KevDemoCameraPreview();

    bool open(int nDevice = 0);
    void close();

signals:

    void sig_receiveFrame(cv::Mat rMat);

public slots:

    void slot_updateFrame();
};

#endif // _KEV_DEMO_CAMERA_PREVIEW_H_
