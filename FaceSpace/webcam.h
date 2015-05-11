#using <System.dll>
#include <vcclr.h>
#include "opencv2/opencv.hpp"


using namespace System;
using namespace System::ComponentModel;
using namespace cv;

namespace CLIBridge {
    extern IplImage *frame;
    extern CvCapture *capture;
    extern HBITMAP bitmap;

    ref class CaptureReader {
    private:
        BackgroundWorker^ reader;

        void task(Object^ sender, DoWorkEventArgs^ e);
        //static HBITMAP iplImage2DIB(const IplImage* Image);
        static bool IplImage2Bmp(IplImage *pImage, HBITMAP &hBitmap);
    public:
        CaptureReader();
        ~CaptureReader();

        void start();
        void stop();
    };
}

class WebCam {
private:
    gcroot<CLIBridge::CaptureReader^> reader;
    bool isRun = false;
public:
    WebCam();
    ~WebCam();

    static HBITMAP getBitmap();
    bool isRunning();
    void start();
    void stop();
};
