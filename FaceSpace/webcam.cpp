#pragma once

#include <common/stdafx.h>
#include "webcam.h"

#include "assert.h"

HBITMAP WebCam::bitmap = NULL;
bool WebCam::isRun = NULL;
HANDLE WebCam::readerThread;
IplImage* WebCam::frame = NULL;
CvCapture* WebCam::capture = NULL;
HWND WebCam::hWnd = NULL;

void WebCam::setHWND(HWND _hWnd) {
    hWnd = _hWnd;
}

void WebCam::start() {
    if (!isRun) {
        readerThread = CreateThread(NULL, NULL, &readerThreadProc, NULL, NULL, NULL);
        isRun = true;
    }
}

void WebCam::stop() {
    if (isRun) {
        isRun = false;
    }
}

bool WebCam::isRunning() {
    return isRun;
}

HBITMAP WebCam::getBitmap() {
    return bitmap;
}

DWORD WINAPI WebCam::readerThreadProc(HANDLE handle) {
    capture = cvCaptureFromCAM(0);
    //capture = cvCaptureFromFile("http:\\192.168.43.1");
    
    for (; isRun;) {
        frame = cvQueryFrame(capture);
        if (frame && (frame->height > 0 && frame->width > 0)) {
            IplImage2Bmp(frame, bitmap);
            HWND hwnd = GetActiveWindow();
            RECT rect;
            GetClientRect(hwnd, &rect);
            InvalidateRect(hwnd, &rect, false);
            Sleep(50);
        } 
    }
    //cvReleaseCapture(&capture);
    TerminateThread(readerThread, 0);
    return 0;
}

bool WebCam::IplImage2Bmp(IplImage *pImage, HBITMAP &hBitmap) {
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