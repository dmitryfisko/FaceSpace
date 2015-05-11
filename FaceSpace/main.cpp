// FaceSpace.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "resource.h"
#include "webcam.h"
#include <ctime>

#define MAX_LOADSTRING 100
#define PHOTO_BUTTON_RADIUS 35
#define sqr(x) (x)*(x)

// Глобальные переменные:
HINSTANCE hInst;								// текущий экземпляр
TCHAR szTitle[MAX_LOADSTRING];					// Текст строки заголовка
TCHAR szWindowClass[MAX_LOADSTRING];			// имя класса главного окна
WebCam webcam;
HDC memHDC;
HDC memHDC2;
HGDIOBJ memBitmap2;
HANDLE timerThread;
bool isLoading = false;
bool isLoadingExpanding = true;
int clientWidth;
int clientHeight;
HWND hWnd;

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void getButtonTakePhotoXY(int &x, int &y) {
    x = clientWidth - PHOTO_BUTTON_RADIUS * 2;
    y = clientHeight - PHOTO_BUTTON_RADIUS * 2;
}

void displayWebCamFrame(HDC &hdc) {
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
        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255)); //white
        RECT rect;
        GetClientRect(GetActiveWindow(), &rect);
        FillRect(memHDC2, &rect, brush);
        DeleteObject(brush);
    }

    SetStretchBltMode(memHDC, COLORONCOLOR);
    StretchBlt(memHDC2, dX, dY, dWidth, dHeight, 
                memHDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

    int buttonX;
    int buttonY;
    getButtonTakePhotoXY(buttonX, buttonY);
    HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    HPEN oldPen = (HPEN)SelectObject(memHDC2, pen);
    //HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    //HBRUSH brush = CreateSolidBrush(RGB(63, 81, 181));
    HBRUSH brush = CreateSolidBrush(RGB(255, 152, 0));
    HBRUSH oldBrush = (HBRUSH)SelectObject(memHDC2, brush);
    Ellipse(memHDC2, buttonX - PHOTO_BUTTON_RADIUS, buttonY - PHOTO_BUTTON_RADIUS, 
            buttonX + PHOTO_BUTTON_RADIUS, buttonY + PHOTO_BUTTON_RADIUS);
    SelectObject(memHDC2, oldBrush);
    DeleteObject(brush);
    SelectObject(memHDC2, oldPen);
    DeleteObject(pen);

    BitBlt(hdc, 0, 0, clientWidth, clientHeight, memHDC2, 0, 0, SRCCOPY);
        
    SelectObject(memHDC2, oldBitmap2);

    SelectObject(memHDC, oldBitmap);
}

void displayLoading(HDC &hdc, size_t passedTime, vector<COLORREF> &colors) {
    const int CIRCLE_ANIMATION_TIME = 1500;
    HGDIOBJ tempBitmap;

    int buttonX;
    int buttonY;
    getButtonTakePhotoXY(buttonX, buttonY);

    tempBitmap = SelectObject(memHDC2, memBitmap2);
    BitBlt(memHDC2, 0, 0, clientWidth, clientHeight, hdc, 0, 0, SRCCOPY);

    double maxDist = sqrt((double)sqr(buttonX) + sqr(buttonY)) * 1.1;
    int colorInd = (passedTime / CIRCLE_ANIMATION_TIME) % colors.size();
    passedTime %= CIRCLE_ANIMATION_TIME;
    int radius = ((double)passedTime / CIRCLE_ANIMATION_TIME) * maxDist;


    HPEN pen = CreatePen(PS_NULL, 0, colors[colorInd]);
    HPEN oldPen = (HPEN)SelectObject(memHDC2, pen);
    //HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
    //HBRUSH brush = CreateSolidBrush(RGB(63, 81, 181));
    HBRUSH brush = CreateSolidBrush(colors[colorInd]);
    HBRUSH oldBrush = (HBRUSH)SelectObject(memHDC2, brush);
    Ellipse(memHDC2, buttonX - radius, buttonY - radius,
            buttonX + radius, buttonY + radius);
    SelectObject(memHDC2, oldBrush);
    DeleteObject(brush);
    SelectObject(memHDC2, oldPen);
    DeleteObject(pen);

    BitBlt(hdc, 0, 0, clientWidth, clientHeight, memHDC2, 0, 0, SRCCOPY);

    SelectObject(memHDC2, tempBitmap);
}

void displayUI(HDC &hdc) {
    displayWebCamFrame(hdc);
}

DWORD WINAPI timerThreadProc(HANDLE handle) {
    Sleep(50);
    const int fps = 100;
    
    HDC hdc = GetDC(hWnd);
    int delay = 1000 / fps;

    vector<COLORREF> colors;
    colors.push_back(RGB(255, 152, 0));
    colors.push_back(RGB(100, 221, 23));
    colors.push_back(RGB(255, 235, 59));

    int passedTime = 0;
    size_t startTime = clock();
    for (;;) {
        displayLoading(hdc, clock() - startTime, colors);
        Sleep(delay);
    }

    DeleteDC(hdc);
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
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
    int clickX;
    int clickY;
    int buttonX;
    int buttonY;

    //Mat mat;
    //CvCapture *capture;
	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Разобрать выбор в меню:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
    case WM_ERASEBKGND:
        return 1;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
        displayUI(hdc);
        EndPaint(hWnd, &ps);
		break;
    case WM_SIZE:
        hdc = BeginPaint(hWnd, &ps);
        memHDC = CreateCompatibleDC(hdc);
        memHDC2 = CreateCompatibleDC(hdc);
        
        RECT rect;
        GetClientRect(hWnd, &rect);
        clientWidth = rect.right - rect.left;
        clientHeight = rect.bottom - rect.top;
        memBitmap2 = ::CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
        EndPaint(hWnd, &ps);
        break;
    case WM_LBUTTONUP:
        clickX = LOWORD(lParam);
        clickY = HIWORD(lParam);
        getButtonTakePhotoXY(buttonX, buttonY);
        if (sqr(buttonX - clickX) + sqr(buttonX - clickX) <= sqr(PHOTO_BUTTON_RADIUS)) {
            if (webcam.isRunning()) {
                webcam.stop();
                timerThread = CreateThread(NULL, NULL, &timerThreadProc, NULL, NULL, NULL);
            } else {
                webcam.start();
            }
        }
        break;
	case WM_DESTROY:
        DeleteDC(memHDC2);
        DeleteDC(memHDC);
        TerminateThread(timerThread, 0); 
        PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
