// FaceSpace.cpp: определяет точку входа для приложения.
//

#include <common/stdafx.h>
#include <common/facedetector.h>
#include <common/classifier.h>
#include "resource.h"
#include "webcam.h"

#include <ctime>
#include <locale>
#include <codecvt>

#define MAX_LOADSTRING 100
#define PHOTO_BUTTON_RADIUS 41
#define CIRCLE_ANIMATION_TIME 1250

#define FRAME_BORDER_WIDTH 3
#define TEXT_SIZE 25
#define POLYGON_TRIANGLE_HEIGHT 4

#define sqr(x) (x)*(x)

// Глобальные переменные:
HINSTANCE hInst;								// текущий экземпляр
TCHAR szTitle[MAX_LOADSTRING];					// Текст строки заголовка
TCHAR szWindowClass[MAX_LOADSTRING];			// имя класса главного окна
WebCam webcam;
vector<Rect> faces;
vector<Classifier::Profile> profiles;
HDC memHDC;
HDC memHDC2;
HGDIOBJ memBitmap2;
HANDLE loadingThread;
HANDLE facedetectionThread;
bool isLoadingRunning = false;
bool isLoadingExpanding = true;
int clientWidth;
int clientHeight;
HWND globalHwnd;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: разместите код здесь.
	MSG msg;
	HACCEL hAccelTable;

	// Инициализация глобальных строк
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FACESPACE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FACESPACE));

    webcam.setHWND(globalHwnd);
    webcam.start();
    // Цикл основного сообщения:
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
    webcam.stop();

	return (int) msg.wParam;
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  НАЗНАЧЕНИЕ: регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = 0; //CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FACESPACE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground =  NULL;
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FACESPACE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   НАЗНАЧЕНИЕ: сохраняет обработку экземпляра и создает главное окно.
//
//   КОММЕНТАРИИ:
//
//        В данной функции дескриптор экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится на экран главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   globalHwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!globalHwnd) {
      return FALSE;
   }

   ShowWindow(globalHwnd, nCmdShow);
   UpdateWindow(globalHwnd);

   return TRUE;
}

void getButtonTakePhotoXY(int &x, int &y) {
    x = clientWidth - PHOTO_BUTTON_RADIUS * 2 - 8;
    y = clientHeight - PHOTO_BUTTON_RADIUS * 2 - 8;
}


std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

void displayWebCamFrame(HDC &hdc, bool isJustMemorizeInMem2) {
    HBITMAP hBitmap;
    hBitmap = webcam.getBitmap();
    if (hBitmap == NULL) {

        return;
    }
    BITMAP bitmap;
    HGDIOBJ oldBitmap;
    HGDIOBJ oldBitmap2;
    
    oldBitmap = SelectObject(memHDC, hBitmap);

    GetObject(hBitmap, sizeof(bitmap), &bitmap);
    int bWidth = bitmap.bmWidth;
    int bHeight = bitmap.bmHeight;
    
    int dX;
    int dY;
    int dWidth;
    int dHeight;

    oldBitmap2 = SelectObject(memHDC2, memBitmap2);

    double wRatio = (double) clientWidth / clientHeight;
    double bRatio = (double) bWidth / bHeight;
    if (bRatio < wRatio) {
        dHeight = clientHeight;
        dWidth = dHeight * bRatio;
    } else {
        dWidth = clientWidth;
        dHeight = dWidth / bRatio;
    }

    dX = (clientWidth - dWidth + 1) / 2;
    dY = (clientHeight - dHeight + 1) / 2;

    if (!dX || !dY) {
        //HBRUSH brush = CreateSolidBrush(RGB(139, 195, 74)); //green
        //HBRUSH brush = CreateSolidBrush(RGB(63, 81, 181)); //dark blue
        HBRUSH brush = CreateSolidBrush(RGB(255, 152, 0)); //white
        RECT rect;
        GetClientRect(globalHwnd, &rect);
        FillRect(memHDC2, &rect, brush);
        DeleteObject(brush);
    }

    SetStretchBltMode(memHDC, COLORONCOLOR);
    StretchBlt(memHDC2, dX, dY, dWidth, dHeight, 
                memHDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

    double scale = (double)dWidth / bitmap.bmWidth;
    //HPEN linePen = CreatePen(PS_SOLID, 5, RGB(238, 255, 65));
    HPEN linePen = CreatePen(PS_SOLID, FRAME_BORDER_WIDTH, RGB(255, 255, 255));
    HPEN oldLinePen = (HPEN)SelectObject(memHDC2, linePen);
    for (int i = 0; i < faces.size(); ++i) {
        Rect scaledFace = FaceDetector::scaleRect(faces[i], scale);
        Rect face = FaceDetector::scaleRectSize(scaledFace, 0.7, INT_MAX, INT_MAX);
        std::wstring wUserName = s2ws(profiles[i].userName);
        int labelOffset = POLYGON_TRIANGLE_HEIGHT + FRAME_BORDER_WIDTH;
        int labelHeight = min((double)40, face.y * 0.18);

        int fontSize = max((double)1, (labelHeight - labelOffset) * 0.85);
        int rectOffset = wUserName.length() * fontSize / 3.2;

        Rectangle(memHDC2, 
                  dX + face.x + face.width / 2 - rectOffset,
                  dY + face.y - labelHeight,
                  dX + face.x + face.width / 2 + rectOffset,
                  dY + face.y - labelOffset);
        SetTextAlign(memHDC2, TA_CENTER);

        
        int textOffset = (labelHeight - labelOffset) * 0.9 + labelOffset;
        

        HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD,
                                 0, 0, 0, 0, 0, 0, 2, 0, L"SYSTEM_FIXED_FONT");
        HFONT hTmp = (HFONT)SelectObject(memHDC2, hFont);
        SetTextColor(memHDC2, RGB(255, 152, 0));
        TextOut(memHDC2, 
                dX + face.x + face.width / 2, 
                dY + face.y - textOffset,
                wUserName.c_str(), wUserName.length());
        DeleteObject(SelectObject(memHDC2, hTmp));

        POINT points[3];
        points[0].x = dX + face.x + face.width / 2;
        points[0].y = dY + face.y - FRAME_BORDER_WIDTH;
        points[1].x = dX + face.x + face.width / 2 - 5;
        points[1].y = dY + face.y - (POLYGON_TRIANGLE_HEIGHT + FRAME_BORDER_WIDTH);
        points[2].x = dX + face.x + face.width / 2 + 5;
        points[2].y = dY + face.y - (POLYGON_TRIANGLE_HEIGHT + FRAME_BORDER_WIDTH);
        Polygon(memHDC2, points, 3);
        MoveToEx(memHDC2, dX + face.x, dY + face.y, NULL);
        LineTo(memHDC2, dX + face.x + face.width, dY + face.y);
        LineTo(memHDC2, dX + face.x + face.width, dY + face.y + face.height);
        LineTo(memHDC2, dX + face.x, dY + face.y + face.height);
        LineTo(memHDC2, dX + face.x, dY + face.y);
    }
    SelectObject(memHDC2, oldLinePen);
    DeleteObject(linePen);

    int buttonX;
    int buttonY;
    getButtonTakePhotoXY(buttonX, buttonY);
    HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(memHDC2, pen);
    //HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    //HBRUSH brush = CreateSolidBrush(RGB(63, 81, 181));
    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH oldBrush = (HBRUSH)SelectObject(memHDC2, brush);
    Ellipse(memHDC2, buttonX - PHOTO_BUTTON_RADIUS, buttonY - PHOTO_BUTTON_RADIUS,
            buttonX + PHOTO_BUTTON_RADIUS, buttonY + PHOTO_BUTTON_RADIUS);
    SelectObject(memHDC2, oldBrush);
    DeleteObject(brush);
    brush = CreateSolidBrush(RGB(255, 152, 0));
    oldBrush = (HBRUSH)SelectObject(memHDC2, brush);
    Ellipse(memHDC2, buttonX - PHOTO_BUTTON_RADIUS + 3, buttonY - PHOTO_BUTTON_RADIUS + 3,
            buttonX + PHOTO_BUTTON_RADIUS - 3, buttonY + PHOTO_BUTTON_RADIUS - 3);
    SelectObject(memHDC2, oldBrush);
    DeleteObject(brush);
    brush = CreateSolidBrush(RGB(255, 255, 255));
    oldBrush = (HBRUSH)SelectObject(memHDC2, brush);
    Ellipse(memHDC2, buttonX - PHOTO_BUTTON_RADIUS + 6, buttonY - PHOTO_BUTTON_RADIUS + 6,
            buttonX + PHOTO_BUTTON_RADIUS - 6, buttonY + PHOTO_BUTTON_RADIUS - 6);
    SelectObject(memHDC2, oldBrush);
    DeleteObject(brush);
    SelectObject(memHDC2, oldPen);
    DeleteObject(pen);

    if (!isJustMemorizeInMem2) {
        BitBlt(hdc, 0, 0, clientWidth, clientHeight, memHDC2, 0, 0, SRCCOPY); 
    } 
    SelectObject(memHDC2, oldBitmap2);
        
    SelectObject(memHDC, oldBitmap);
}

bool displayLoading(HDC &hdc, size_t passedTime, bool isReversed, bool isDuplicateLastColor,
                    vector<COLORREF> &colors) {
    HGDIOBJ tempBitmap = NULL;
    bool isLoadFromMemorized = isReversed;

    int buttonX;
    int buttonY;
    getButtonTakePhotoXY(buttonX, buttonY);

    tempBitmap = SelectObject(memHDC2, memBitmap2);
    if (!isLoadFromMemorized) {
        BitBlt(memHDC2, 0, 0, clientWidth, clientHeight, hdc, 0, 0, SRCCOPY);
    }

    double maxDist = sqrt((double)sqr(buttonX) + sqr(buttonY)) * 1.1;
    int colorInd = (passedTime / CIRCLE_ANIMATION_TIME) % colors.size();
    passedTime %= CIRCLE_ANIMATION_TIME;
    int radius;
    if (isReversed) {
        colorInd = 0;
        radius = (1 - ((double)passedTime / CIRCLE_ANIMATION_TIME)) * maxDist;
    } else {
        radius = ((double)passedTime / CIRCLE_ANIMATION_TIME) * maxDist;
    }

    COLORREF color;
    if (isReversed || isDuplicateLastColor) {
        color = RGB(255, 255, 255);
    } else {
        color = colors[colorInd];
    }

    HPEN pen = CreatePen(PS_NULL, 0, color);
    HPEN oldPen = (HPEN)SelectObject(memHDC2, pen);
    //HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    //HBRUSH brush = CreateSolidBrush(RGB(63, 81, 181));
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memHDC2, brush);
    Ellipse(memHDC2, buttonX - radius, buttonY - radius,
            buttonX + radius, buttonY + radius);
    SelectObject(memHDC2, oldBrush);
    DeleteObject(brush);
    SelectObject(memHDC2, oldPen);
    DeleteObject(pen);

    BitBlt(hdc, 0, 0, clientWidth, clientHeight, memHDC2, 0, 0, SRCCOPY);

    SelectObject(memHDC2, tempBitmap);

    if (isReversed) {
        return radius <= PHOTO_BUTTON_RADIUS;
    } else {
        return ((double)passedTime / CIRCLE_ANIMATION_TIME) >= 0.92;
    }
}

void displayUI(HDC &hdc) {
    displayWebCamFrame(hdc, false);
}

DWORD WINAPI loadingThreadProc(HANDLE handle) {
    Sleep(50);
    const int fps = 100;
    
    HDC hdc = GetDC(globalHwnd);
    int delay = 1000 / fps;

    LONG windowParams = GetWindowLong(globalHwnd, GWL_STYLE)&~WS_SIZEBOX;
    SetWindowLong(globalHwnd, GWL_STYLE, windowParams);

    vector<COLORREF> colors;
    colors.push_back(RGB(255, 152, 0));
    colors.push_back(RGB(100, 221, 23));
    colors.push_back(RGB(255, 235, 59));
    colors.push_back(RGB(3, 169, 244));
    //colors.push_back(RGB(0, 150, 136));

    int passedTime = 0;
    size_t startTime = clock();
    bool reversed = false;
    bool isDuplicateLastColor = false;
    for (;;) {
        size_t passedTime = clock() - startTime;
        if (!reversed || !isDuplicateLastColor) {
            bool isLimitReached = displayLoading(hdc, passedTime,
                                                 reversed, isDuplicateLastColor, colors);
            if (isLimitReached && !isLoadingRunning) {
                size_t difTime = passedTime;
                difTime %= CIRCLE_ANIMATION_TIME;
                startTime -= CIRCLE_ANIMATION_TIME - difTime;
                if (!isDuplicateLastColor) {
                    if (passedTime / CIRCLE_ANIMATION_TIME > 2) {
                        isDuplicateLastColor = true;
                    }
                } else {
                    reversed = true;
                }
            } else if (isLimitReached && !isLoadingRunning) {
                isDuplicateLastColor = true;
            }
        } else {
            displayWebCamFrame(hdc, true);
            bool isLimitReached = displayLoading(hdc, clock() - startTime, 
                                                 reversed, false, colors);
            if (isLimitReached) {
                windowParams = GetWindowLong(globalHwnd, GWL_STYLE)|WS_SIZEBOX;
                SetWindowLong(globalHwnd, GWL_STYLE, windowParams);
                
                displayWebCamFrame(hdc, false);
                DeleteDC(hdc);
                TerminateThread(facedetectionThread, 0);
                return 0;
            }
        }
        Sleep(delay);
    }

    return 0;
}

DWORD WINAPI facedetectionThreadProc(HANDLE handle) {
    Sleep(50);
    FaceDetector detector;
    Classifier classifier;
    Mat frame = webcam.frame;
    faces = detector.detect(frame, FaceDetector::FindAllFaces);
    profiles = classifier.getProfiles(frame, faces);

    isLoadingRunning = false;
    Sleep(1000);
    TerminateThread(facedetectionThread, 0);
    return 0;
}


//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  НАЗНАЧЕНИЕ:  обрабатывает сообщения в главном окне.
//
//  WM_COMMAND	- обработка меню приложения
//  WM_PAINT	-Закрасить главное окно
//  WM_DESTROY	 - ввести сообщение о выходе и вернуться.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
    int clickX;
    int clickY;
    int buttonX;
    int buttonY;

    //Mat mat;
    //CvCapture *capture;
	switch (message){
    case WM_ERASEBKGND:
        return 1;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
        displayUI(hdc);
        EndPaint(hWnd, &ps);
		break;
    case WM_SIZE:
        hdc = GetDC(globalHwnd);
        DeleteDC(memHDC);
        DeleteDC(memHDC2);
        memHDC = CreateCompatibleDC(hdc);
        memHDC2 = CreateCompatibleDC(hdc);
        
        RECT rect;
        GetClientRect(hWnd, &rect);
        clientWidth = rect.right - rect.left;
        clientHeight = rect.bottom - rect.top;
        memBitmap2 = ::CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
        displayUI(hdc);
        break;
    case WM_LBUTTONUP:
        clickX = LOWORD(lParam);
        clickY = HIWORD(lParam);
        getButtonTakePhotoXY(buttonX, buttonY);
        if (sqr(buttonX - clickX) + sqr(buttonY - clickY) <= sqr(PHOTO_BUTTON_RADIUS)) {
            if (webcam.isRunning()) {
                webcam.stop();
                isLoadingRunning = true;
                loadingThread = CreateThread(NULL, NULL, &loadingThreadProc, NULL, NULL, NULL);
                facedetectionThread = CreateThread(NULL, NULL, &facedetectionThreadProc, NULL, NULL, NULL);
            } else {
                faces.clear();
                webcam.start();
            }
        }
        break;
	case WM_DESTROY:
        DeleteDC(memHDC2);
        DeleteDC(memHDC);
        TerminateThread(loadingThread, 0); 
        TerminateThread(facedetectionThread, 0);
        webcam.stop();
        PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
