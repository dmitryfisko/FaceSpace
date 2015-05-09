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
            //bitmap = iplImage2DIB(frame);
            imshow("sdf", image);
            cvWaitKey(10);
        } 
    }
    cvReleaseImage(&frame);
    e->Cancel = true;
}

HBITMAP CLIBridge::CaptureReader::iplImage2DIB(const IplImage* Image) {
    int bpp = Image->nChannels * 8;
    assert(Image->width >= 0 && Image->height >= 0 &&
           (bpp == 8 || bpp == 24 || bpp == 32));
    CvMat dst;
    void* dst_ptr = 0;
    HBITMAP hbmp = NULL;
    unsigned char buffer[sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)];
    BITMAPINFO* bmi = (BITMAPINFO*)buffer;
    BITMAPINFOHEADER* bmih = &(bmi->bmiHeader);

    ZeroMemory(bmih, sizeof(BITMAPINFOHEADER));
    bmih->biSize = sizeof(BITMAPINFOHEADER);
    bmih->biWidth = Image->width;
    bmih->biHeight = Image->origin ? abs(Image->height) : -abs(Image->height);
    bmih->biPlanes = 1;
    bmih->biBitCount = bpp;
    bmih->biCompression = BI_RGB;

    if (bpp == 8) {
        RGBQUAD* palette = bmi->bmiColors;
        int i;
        for (i = 0; i < 256; i++) {
            palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue
                = (BYTE) i;
            palette[i].rgbReserved = 0;
        }
    }

    hbmp = CreateDIBSection(NULL, bmi, DIB_RGB_COLORS, &dst_ptr, 0, 0);
    cvInitMatHeader(&dst, Image->height, Image->width, CV_8UC3,
                    dst_ptr, (Image->width * Image->nChannels + 3) & -4);
    cvConvertImage(Image, &dst, Image->origin ? CV_CVTIMG_FLIP : 0);

    return hbmp;
}