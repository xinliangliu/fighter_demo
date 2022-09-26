#include <windows.h>
#include <stdio.h>

#define MULTIPLIER				1
#define	RESOLUTIONSIZE			0x0100
#define COORDINATESIDELENGTH	(RESOLUTIONSIZE * MULTIPLIER)
#define CANVASSIDELENGTH		((RESOLUTIONSIZE + 0x40) * MULTIPLIER)
#define ORIGINSIDELENGTH		16
#define DOTSPLITNUM				4
#define DZLOOPNUM				3

typedef struct tagPoint
{
	INT x;
	INT y;
} Point;

enum Mode
{
	MODE_NULL = -1,
	MODE_STAY,
	MODE_PUNCH,
	MODE_KICK,
	MODE_END
};

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
HDC 		manDC;
HBITMAP 	bmpBack;
WORD		order;
BOOL		init;
BOOL		keyvalid;
enum Mode	mode[2];
HBITMAP		man;

int WINAPI
Init(HWND hwnd)
{
	hDC = GetDC(hwnd);
	memDC = CreateCompatibleDC(hDC);
	bmpBack = CreateCompatibleBitmap(hDC, CANVASSIDELENGTH, CANVASSIDELENGTH);
	order = 0;
	init = TRUE;
	keyvalid = TRUE;
	mode[0] = mode[1] = MODE_STAY;
	manDC = CreateCompatibleDC(hDC);
	man = (HBITMAP)LoadImage(NULL, "all.dz", IMAGE_BITMAP, DZLOOPNUM * 100, MODE_END * 100, LR_LOADFROMFILE);
	
	SetTimer(hwnd, 1, 100, NULL);
	
	SelectObject(memDC, bmpBack);
	SelectObject(manDC, man);
	return TRUE;
}

int WINAPI
Release(HWND hwnd)
{	
	DeleteObject(bmpBack);
	DeleteObject(man);
	DeleteDC(memDC);
	DeleteDC(manDC);
	ReleaseDC(hwnd, hDC);
	
	KillTimer(hwnd, 1);
	
	return TRUE;
}

int WINAPI
Paint()
{
	PaintBack();
	PaintDemo();
	
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
	
	if (init)
	{
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
		
		init = FALSE;
	}
	
	BitBlt(hDC, 0, 0, CANVASSIDELENGTH, CANVASSIDELENGTH, memDC, 0, 0, SRCCOPY);
	
	return TRUE;
}

int WINAPI
PaintDemo()
{
	TransparentBlt(hDC, 30, 30, 100, 100, manDC, order * 100, mode[1] * 100, 100, 100, RGB(255,255,255));
	
	order++;
	order %= DZLOOPNUM;
	
	if (!order && mode[1])
	{
		mode[0] = mode[1];
		mode[1] = MODE_STAY;
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
			break;
		case WM_TIMER:
			Paint();
			break;
		case WM_KEYDOWN:
			printf("down - %x\n", (unsigned int)wParam);
			fflush(stdout);
			if (wParam == 0x4A && !mode[1] && keyvalid)
			{
				mode[0] = mode[1];
				mode[1] = MODE_PUNCH;
				order = 0;
				keyvalid = FALSE;
			}
			else
			if (wParam == 0x4B && !mode[1] && keyvalid)
			{
				mode[0] = mode[1];
				mode[1] = MODE_KICK;
				order = 0;
				keyvalid = FALSE;
			}
			break;
		case WM_KEYUP:
			printf("up - %x\n", (unsigned int)wParam);
			fflush(stdout);
			if (wParam == 0x4A)
			{
				keyvalid = TRUE;
			}
			else
			if (wParam == 0x4B)
			{
				keyvalid = TRUE;
			}
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
						TEXT ("Animation Demo"),
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