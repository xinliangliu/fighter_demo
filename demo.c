#include <windows.h>
#include <stdio.h>

#define MULTIPLIER				1
#define	RESOLUTIONSIZE				0x0100
#define COORDINATESIDELENGTH	(RESOLUTIONSIZE * MULTIPLIER)
#define CANVASSIDELENGTH		((RESOLUTIONSIZE + 0x40) * MULTIPLIER)
#define ORIGINSIDELENGTH		16
#define DOTSPLITNUM				4

typedef struct tagPoint
{
	INT x;
	INT y;
} Point;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI Init(HWND);
int WINAPI Release(HWND);
int WINAPI Paint();
int WINAPI PaintBack();
int WINAPI PaintDemo();

int WINAPI LG2DC(Point *);
int WINAPI DC2LG(Point *);
int WINAPI DrawRectangle(HDC, Point, Point);
int WINAPI FillRectangle(HDC, Point, Point, HBRUSH);
int WINAPI DrawScatter(HDC, Point, int, HBRUSH);
int WINAPI DrawLine(HDC, Point, Point);

TCHAR szClassName[] = TEXT("LCC");
HDC 		hDC;
HDC 		memDC;
HBITMAP 	bmpBack;
WORD		order;
CHAR		flag;

int WINAPI
Init(HWND hwnd)
{
	hDC = GetDC(hwnd);
	memDC = CreateCompatibleDC(hDC);
	bmpBack = CreateCompatibleBitmap(hDC, CANVASSIDELENGTH, CANVASSIDELENGTH);
	order = 0;
	flag = 0;
	
	SelectObject(memDC,bmpBack);
	return TRUE;
}

int WINAPI
Release(HWND hwnd)
{	
	DeleteObject(bmpBack);
	DeleteDC(memDC);
	ReleaseDC(hwnd, hDC);
	
	return TRUE;
}

int WINAPI
Paint()
{
	BitBlt(hDC, 0, 0, CANVASSIDELENGTH, CANVASSIDELENGTH, memDC, 0, 0, SRCCOPY);
	return TRUE;
}

int WINAPI
LG2DC(Point *point)
{
	point->x = ORIGINSIDELENGTH + point->x * MULTIPLIER;
	point->y = COORDINATESIDELENGTH + ORIGINSIDELENGTH - point->y * MULTIPLIER;
	
	return TRUE;
}

int WINAPI
DC2LG(Point *point)
{
	point->x = (point->x - ORIGINSIDELENGTH) / MULTIPLIER;
	point->y = (COORDINATESIDELENGTH + ORIGINSIDELENGTH - point->y) / MULTIPLIER;
	
	return TRUE;
}

int WINAPI
DrawRectangle(HDC hdc, Point op, Point tp)
{
	LG2DC(&op);
	LG2DC(&tp);
	
	Rectangle(hdc, op.x, op.y, tp.x, tp.y);
	
	return TRUE;
}

int WINAPI
FillRectangle(HDC hdc, Point op, Point tp, HBRUSH brush)
{
	RECT 		rcClient;
	
	LG2DC(&op);
	LG2DC(&tp);
	
	rcClient.left = op.x;
	rcClient.top = op.y;
	rcClient.right = tp.x;
	rcClient.bottom = tp.y;
	
	FillRect(hdc, &rcClient, brush);
	
	return TRUE;
}

int WINAPI
DrawScatter(HDC hdc, Point pt, int side, HBRUSH brush)
{
	Point psrc;
	Point pdst;
	
	psrc.x = pt.x - side;
	psrc.y = pt.y + side;
	pdst.x = pt.x + side;
	pdst.y = pt.y - side;
	
	FillRectangle(hdc, psrc, pdst, brush);
	
	return TRUE;
}

int WINAPI
DrawLine(HDC hdc, Point op, Point tp)
{
	LG2DC(&op);
	LG2DC(&tp);
	
	MoveToEx(hdc, op.x, op.y, NULL);
	LineTo(hdc, tp.x , tp.y);
	
	return TRUE;
}

int WINAPI 
PaintBack()
{
	HPEN 		penBackSolidBlack;
	HPEN 		penBackDotBlack;
	HBRUSH 		brushBack;
	RECT 		rcClient;
	Point		psrc, pdst;
	int 		i;
	
	penBackSolidBlack = CreatePen(PS_SOLID, 3, RGB(0,0,0));
	penBackDotBlack = CreatePen(PS_DOT, 1, RGB(0,0,0));
	brushBack = CreateSolidBrush(RGB(255, 255, 255));
	
	rcClient.left = 0;
	rcClient.top = 0;
	rcClient.right = CANVASSIDELENGTH;
	rcClient.bottom = CANVASSIDELENGTH;
    FillRect(memDC, &rcClient, brushBack);
	
    SelectObject(memDC,penBackSolidBlack);
	psrc.x = 0;
	psrc.y = 0;
	pdst.x = RESOLUTIONSIZE - 1;
	pdst.y = RESOLUTIONSIZE - 1;
	DrawRectangle(memDC, psrc, pdst);
	
	SelectObject(memDC,penBackDotBlack);
	for (i = RESOLUTIONSIZE / DOTSPLITNUM; i < RESOLUTIONSIZE; i += RESOLUTIONSIZE / DOTSPLITNUM)
	{
		psrc.x = 0;
		psrc.y = i;
		pdst.x = RESOLUTIONSIZE - 1;
		pdst.y = i;
		
		DrawLine(memDC, psrc, pdst);
		
		psrc.x = i;
		psrc.y = 0;
		pdst.x = i;
		pdst.y = RESOLUTIONSIZE - 1;
		
		DrawLine(memDC, psrc, pdst);
	}
	
	DeleteObject(penBackSolidBlack);
	DeleteObject(penBackDotBlack);
	DeleteObject(brushBack);
	
	return TRUE;
}

int WINAPI
PaintDemo()
{
	HBRUSH 	brush;
	Point 	psrc;
	Point 	pdst;
	
	if(flag) brush = CreateSolidBrush(RGB(255, 255, 255));
	else brush = CreateSolidBrush(RGB(255, 255, 0));
	
	psrc.x = (order / 4) * (RESOLUTIONSIZE / DOTSPLITNUM) + 1;
	psrc.y = (order % 4 + 1) * (RESOLUTIONSIZE / DOTSPLITNUM) - 2;
	pdst.x = (order / 4 + 1) * (RESOLUTIONSIZE / DOTSPLITNUM) - 2;
	pdst.y = (order % 4) * (RESOLUTIONSIZE / DOTSPLITNUM) + 1;
	FillRectangle(memDC, psrc, pdst, brush);
	
	if(order >= 15)
	{
		order = 0;
		flag = flag > 0 ? 0 : 1;
	}
	else
	{
		order++;
	}
	
	return TRUE;
}

LRESULT CALLBACK 
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_DESTROY:
			Release(hwnd);
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			Init(hwnd);
			PaintBack();
			break;
		case WM_TIMER:
			PaintDemo();
			Paint();
			break;
		default:
			return DefWindowProc (hwnd, message, wParam, lParam);
	}
	
	return TRUE;
}

int WINAPI
WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR szCmdLine,
		int iCmdShow)
{
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szClassName;
	
	hPrevInstance = hPrevInstance;
	szCmdLine = szCmdLine;

	if(!RegisterClass (&wndclass))
	{
		MessageBox (NULL, TEXT ("RegisterClass Fail!"),
					szClassName, MB_ICONERROR);
		return TRUE;
	}
	
	HWND hwnd;
	hwnd = CreateWindow(szClassName,
						TEXT ("Leukocyte Clustering & Classification"),
						WS_OVERLAPPEDWINDOW,
						ORIGINSIDELENGTH,
						ORIGINSIDELENGTH,
						CANVASSIDELENGTH,
						CANVASSIDELENGTH,
						NULL,
						NULL,
						hInstance,
						NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	SetTimer(hwnd, 1, 100, NULL);
	
	MSG msg = {0};
	while(TRUE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
			if(msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	
	UnregisterClass(szClassName, wndclass.hInstance);
	return msg.wParam;
}
