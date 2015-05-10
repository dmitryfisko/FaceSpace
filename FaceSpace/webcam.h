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

    static Mat getImage(string path) {
        return imread(path.c_str(), -1);
    }

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

public:
    WebCam();
    ~WebCam();

    HBITMAP getBitmap();
    void start();
    void stop();
};
