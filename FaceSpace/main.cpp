// FaceSpace.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "resource.h"
#include "webcam.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;								// текущий экземпляр
TCHAR szTitle[MAX_LOADSTRING];					// Текст строки заголовка
TCHAR szWindowClass[MAX_LOADSTRING];			// имя класса главного окна
WebCam webcam;
HDC memHDC;
HDC memHDC2;
HGDIOBJ memBitmap2;

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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

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

/*void displayWebCamFrame(HDC &hdc) {
    HDC          hdcMem;
    HBITMAP      hbmMem;
    BITMAP bitmap;
    HANDLE       hOld;

    PAINTSTRUCT  ps;
    HDC          hdc;
    
    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = CreateCompatibleBitmap(hdc, win_width, win_height);
    hOld = SelectObject(hdcMem, hbmMem);

    // Здесь рисуем в hdcMem
     = (HBITMAP)::SelectObject(hdcMem, webcam.getBitmap());

    // Выводим построенное  изображение и памяти на экран
    BitBlt(hdc, 0, 0, win_width, win_height, hdcMem, 0, 0, SRCCOPY);

    // Освобождаем память
    SelectObject(hdcMem, hOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
}*/

void displayWebCamFrame(HDC &hdc) {
    HBITMAP hBitmap;
    hBitmap = webcam.getBitmap();
    if (hBitmap == NULL) {
        return;
    }
    BITMAP bitmap;
    HGDIOBJ oldBitmap;
    HGDIOBJ oldBitmap2;
    RECT rect;
    
    oldBitmap = SelectObject(memHDC, hBitmap);

    GetObject(hBitmap, sizeof(bitmap), &bitmap);
    int bWidth = bitmap.bmWidth;
    int bHeight = bitmap.bmHeight;
    
    if (GetClientRect(GetActiveWindow(), &rect)) {
        int dX;
        int dY;
        int dWidth;
        int dHeight;

        int wWidth = rect.right - rect.left;
        int wHeight = rect.bottom - rect.top;

        oldBitmap2 = SelectObject(memHDC2, memBitmap2);

        double wRatio = (double) wWidth / wHeight;
        double bRatio = (double) bWidth / bHeight;
        if (bRatio < wRatio) {
            dHeight = wHeight;
            dWidth = dHeight * bRatio;
        } else {
            dWidth = wWidth;
            dHeight = dWidth / bRatio;
        }

        dX = (wWidth - dWidth + 1) / 2;
        dY = (wHeight - dHeight + 1) / 2;

        if (!dX || !dY) {
            //HBRUSH brush = CreateSolidBrush(RGB(139, 195, 74)); //green
            HBRUSH brush = CreateSolidBrush(RGB(63, 81, 181)); //dark blue
            FillRect(memHDC2, &rect, brush);
            DeleteObject(brush);
        }
        
        SetStretchBltMode(memHDC, COLORONCOLOR);
        StretchBlt(memHDC2, dX, dY, dWidth, dHeight, 
                   memHDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

        BitBlt(hdc, 0, 0, wWidth, wHeight, memHDC2, 0, 0, SRCCOPY);
        
        SelectObject(memHDC2, oldBitmap2);
    }

    SelectObject(memHDC, oldBitmap);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
    int wWidth;
    int wHeight;

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
        displayWebCamFrame(hdc);
		break;
    case WM_SIZE:
        hdc = BeginPaint(hWnd, &ps);
        memHDC = CreateCompatibleDC(hdc);
        memHDC2 = CreateCompatibleDC(hdc);
        
        RECT rect;
        GetClientRect(GetActiveWindow(), &rect);
        wWidth = rect.right - rect.left;
        wHeight = rect.bottom - rect.top;
        memBitmap2 = ::CreateCompatibleBitmap(hdc, wWidth, wHeight);
        EndPaint(hWnd, &ps);
        break;
	case WM_DESTROY:
        DeleteDC(memHDC2);
        DeleteDC(memHDC);
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
