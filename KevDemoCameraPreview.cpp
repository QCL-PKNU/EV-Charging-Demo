#include "KevDemoCameraPreview.h"

//////////////////////////////////////////////////
// Constructor & Destructor Definition
//////////////////////////////////////////////////

KevDemoCameraPreview::KevDemoCameraPreview()
{
    // Create a camera preview
    m_pCameraPreview = new cv::VideoCapture();
}

KevDemoCameraPreview::~KevDemoCameraPreview()
{
    // Close and release a camera preview
    if(m_pCameraPreview != nullptr)
    {
        close();
        delete m_pCameraPreview;
    }
}

//////////////////////////////////////////////////
// Member Function Definition
//////////////////////////////////////////////////

/**
 * @brief This function opens to preview a camera.
 * @param nDevice camera device ID
 * @return if a camera preview is opended well.
 */
bool
KevDemoCameraPreview::open(int nDevice)
{
    if(m_pCameraPreview == nullptr)
    {
        return false;
    }

    // Open a camera preview obejct of the given device ID
    if(m_pCameraPreview->open(nDevice) == false)
    {
        return false;
    }

    // Configure a timer (24~26 frames/sec)
    QTimer *timer = new QTimer(this);
    timer->setInterval(20);
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_updateFrame()));

    // Create a thread
    m_pThread = new QThread();
    timer->start();
    timer->moveToThread(m_pThread);

    // Start the thread
    m_pThread->start();
    return true;
}

/**
 * @brief close This function closes to preview a camera.
 */
void
KevDemoCameraPreview::close()
{
    // Close to preview a camera
    if(m_pCameraPreview->isOpened())
    {
        m_pCameraPreview->release();
    }

    // Close a thread for previewing camera frames
    m_pThread->quit();
    while(!m_pThread->isFinished());
}

//////////////////////////////////////////////////
// Slot Function Definition
//////////////////////////////////////////////////

/**
 * @brief This function is a slot function to be called when a frame receives.
 */
void
KevDemoCameraPreview::slot_updateFrame()
{
    // Capture a video frame
    (*m_pCameraPreview) >> m_rPreviewFrame;

    // Convert to a gray-colored image
    cv::cvtColor(m_rPreviewFrame, m_rPreviewFrame, cv::COLOR_BGR2GRAY);

    // Emit a signal of frame capture
    emit sig_receiveFrame(m_rPreviewFrame);
}
