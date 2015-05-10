#pragma once

#include "stdafx.h"
#include "webcam.h"

#include "assert.h"

IplImage* CLIBridge::frame = 0;
CvCapture* CLIBridge::capture = 0;
HBITMAP CLIBridge::bitmap = NULL;

WebCam::WebCam() {
    reader = gcnew CLIBridge::CaptureReader();
}

WebCam::~WebCam() {
    delete reader;
}

void WebCam::start() {
    CLIBridge::capture = cvCaptureFromCAM(0);
    reader->start();
}

void WebCam::stop() {
    reader->stop();
    cvReleaseCapture(&CLIBridge::capture);
}

HBITMAP WebCam::getBitmap() {
    return CLIBridge::bitmap;
}

CLIBridge::CaptureReader::CaptureReader() {
    reader = gcnew System::ComponentModel::BackgroundWorker();
    reader->WorkerSupportsCancellation = true;
    reader->DoWork += gcnew DoWorkEventHandler(this, &CaptureReader::task);
}

CLIBridge::CaptureReader::~CaptureReader() {
    delete reader;
}

void CLIBridge::CaptureReader::start() {
    reader->RunWorkerAsync();
}

void CLIBridge::CaptureReader::stop() {
    reader->CancelAsync();
}

void CLIBridge::CaptureReader::task(Object^ sender, DoWorkEventArgs^ e) {
    for (; !reader->CancellationPending;) {
        frame = cvQueryFrame(capture);
        Mat image = frame;
        if (!image.empty()) {
            IplImage2Bmp(frame, bitmap);
            InvalidateRect(GetActiveWindow(), NULL, TRUE);
            System::Threading::Thread::Sleep(100);
        } 
    }
    cvReleaseImage(&frame);
    e->Cancel = true;
}

bool CLIBridge::CaptureReader::IplImage2Bmp(IplImage *pImage, HBITMAP &hBitmap) {
    if (pImage && pImage->depth == IPL_DEPTH_8U) {
        uchar buffer[sizeof(BITMAPINFOHEADER) + 1024];
        BITMAPINFO* bmi = (BITMAPINFO*) buffer;
        int bmp_w = pImage->width, bmp_h = pImage->height;

        int width = bmp_w;
        int height = bmp_h;
        int bpp = pImage ? (pImage->depth & 255)*pImage->nChannels : 0;
        int origin = pImage->origin;

        assert(bmi && width >= 0 && height >= 0 && (bpp == 8 || bpp == 24 || bpp == 32));

        BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

        memset(bmih, 0, sizeof(*bmih));
        bmih->biSize = sizeof(BITMAPINFOHEADER);
        bmih->biWidth = width;
        bmih->biHeight = origin ? abs(height) : -abs(height);
        bmih->biPlanes = 1;
        bmih->biBitCount = (unsigned short) bpp;
        bmih->biCompression = BI_RGB;

        if (bpp == 8) {
            RGBQUAD* palette = bmi->bmiColors;
            for (int i = 0; i < 256; i++) {
                palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE) i;
                palette[i].rgbReserved = 0;
            }
        }
        if (bpp == 24) {
            ((DWORD*)bmi->bmiColors)[0] = 0x00FF0000; 
            ((DWORD*)bmi->bmiColors)[1] = 0x0000FF00;
            ((DWORD*)bmi->bmiColors)[2] = 0x000000FF;
        }

        HDC hdc = ::GetDC(NULL);;
        hBitmap = CreateDIBitmap(hdc, bmih, CBM_INIT, pImage->imageData, bmi, DIB_RGB_COLORS);
        ::ReleaseDC(NULL, hdc);
        return true;
    }
    return false;
}