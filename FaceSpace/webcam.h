#include "opencv2/opencv.hpp"

class WebCam {
private:
    static HBITMAP bitmap;
    static bool isRun;
    static HANDLE readerThread;
    static HWND hWnd;

    static DWORD WINAPI readerThreadProc(HANDLE handle);
    static bool IplImage2Bmp(IplImage *pImage, HBITMAP &hBitmap);
public:
    static IplImage *frame;
    static CvCapture *capture;

    void setHWND(HWND _hWnd);
    HBITMAP getBitmap();
    bool isRunning();
    void start();
    void stop();
};
