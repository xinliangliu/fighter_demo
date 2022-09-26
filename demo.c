#include <windows.h>
#include <stdio.h>

#define MULTIPLIER				1
#define	RESOLUTIONSIZE			0x0100
#define COORDINATESIDELENGTH	(RESOLUTIONSIZE * MULTIPLIER)
#define CANVASSIDELENGTH		((RESOLUTIONSIZE + 0x40) * MULTIPLIER)
#define ORIGINSIDELENGTH		16
#define DOTSPLITNUM				4
#define DZLOOPNUM				9

#define	STAY	(0x0)
#define	JUMP	(0x80)
#define	SQUAT	(0x40)
#define RETREAT	(0x20)
#define ADVANCE	(0x10)

#define JUMPRETREAT	(JUMP | RETREAT)
#define JUMPADVANCE	(JUMP | ADVANCE)

typedef struct tagPoint
{
	int x;
	int y;
} Point;

typedef struct tagAction
{
	unsigned char id;
	unsigned char action;
} Action;
				
typedef struct tagModeFrame
{
	unsigned char mode;
	unsigned char len;
} ModeFrame;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI Init(HWND);
int WINAPI Release(HWND);
int WINAPI Paint();
int WINAPI PaintBack();
int WINAPI PaintDemo();
int WINAPI ChangeFrameLocation();

int WINAPI LG2DC(Point *);
int WINAPI DC2LG(Point *);
int WINAPI DrawRectangle(HDC, Point, Point);
int WINAPI FillRectangle(HDC, Point, Point, HBRUSH);
int WINAPI DrawScatter(HDC, Point, int, HBRUSH);
int WINAPI DrawLine(HDC, Point, Point);
int WINAPI GetAction(unsigned char, unsigned char *);
int WINAPI GetModeID(unsigned char, unsigned char *);

TCHAR szClassName[] = TEXT("LCC");
HDC 		hDC;
HDC 		memDC;
HDC 		manDC;
HBITMAP 	bmpBack;
BOOL		init;
HBITMAP		man;

unsigned char	curmode;
unsigned char	nextmode;
unsigned char	action;
unsigned char	modeid;
unsigned char	curmodeid;
unsigned char	nextmodeid;
unsigned char	mv;

int		maxloc[4];
int		curloc[2];

Action act[] = {{'W', JUMP},
				{'S', SQUAT},
				{'A', RETREAT},
				{'D', ADVANCE},
				{0x0, STAY}};

ModeFrame mf[] = {{JUMP,	8},
				  {SQUAT,	5},
				  {RETREAT,	3},
				  {ADVANCE,	3},
				  {JUMPRETREAT,	8},
				  {JUMPADVANCE,	8},
				  {STAY,	3},
				  {0xFF,	0}};
				  
Point shift[][DZLOOPNUM] = 
				{{{0, -40}, {0, -30}, {0, -20}, {0, -10}, {0, 10}, {0, 20}, {0, 30}, {0, 40}},
				 {{0, 20}, {0, 25}, {0, 0}, {0, 0}, {0, -45}},
				 {{-10, 0}, {-10, 0}, {-10, 0}},
				 {{10, 0}, {10, 0}, {10, 0}},
				 {{-10, -40}, {-10, -30}, {-10, -20}, {-10, -10}, {-10, 10}, {-10, 20}, {-10, 30}, {-10, 40}},
				 {{10, -40}, {10, -30}, {10, -20}, {10, -10}, {10, 10}, {10, 20}, {10, 30}, {10, 40}},
				 {{0, 0}, {0, 0}, {0, 0}}};

int WINAPI
Init(HWND hwnd)
{
	hDC = GetDC(hwnd);
	memDC = CreateCompatibleDC(hDC);
	bmpBack = CreateCompatibleBitmap(hDC, CANVASSIDELENGTH, CANVASSIDELENGTH);
	init = TRUE;
	curmode = 0;
	nextmode = 0;
	action = STAY;
	modeid = 0;
	curmodeid = 6;
	nextmodeid = 6;
	mv = 0;
	maxloc[0] = -100;
	maxloc[1] = 86;
	maxloc[2] = -1000;
	maxloc[3] = 0;
	curloc[0] = 0;
	curloc[1] = 0;
	manDC = CreateCompatibleDC(hDC);
	man = (HBITMAP)LoadImage(NULL, "all.dz", IMAGE_BITMAP, DZLOOPNUM * 100, 7 * 100, LR_LOADFROMFILE);
	
	SetTimer(hwnd, 1, 40, NULL);
	
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
GetAction(unsigned char id, unsigned char *action)
{
	unsigned char i = 0;
	
	while(act[i].id != 0x0)
	{
		if (act[i].id == id)
		{
			*action = act[i].action;
			return TRUE;
		}
		else i++;
	}
	
	return FALSE;
}

int WINAPI
GetModeID(unsigned char mode, unsigned char *modeid)
{
	unsigned char i = 0;
	
	while(mf[i].mode != 0xFF)
	{
		if (mf[i].mode == mode)
		{
			*modeid = i;
			return TRUE;
		}
		else i++;
	}
	
//	for compatibility
	i = 0;
	while(mf[i].mode != 0xFF)
	{
		if ((mf[i].mode & mode) == mf[i].mode)
		{
			*modeid = i;
			return TRUE;
		}
		else i++;
	}
	
	return FALSE;
}

int WINAPI
ChangeFrameLocation()
{
	if (mv >= mf[curmodeid].len)
	{
		mv = 0;
		curmode = mf[nextmodeid].mode;
		curmodeid = nextmodeid;
	}
	
	curloc[0] += shift[curmodeid][mv].x;
	if (curloc[0] < maxloc[0]) curloc[0] = maxloc[0];
	if (curloc[0] > maxloc[1]) curloc[0] = maxloc[1];
	curloc[1] += shift[curmodeid][mv].y;
	
	return TRUE;
}

int WINAPI
PaintDemo()
{
	int loc[2];
	loc[0] = 0;
	loc[0] = 0;
	if (ChangeFrameLocation())
	{
		loc[0] = curloc[0];
		loc[1] = curloc[1];
		if (curloc[1] < maxloc[2]) loc[1] = maxloc[2];
		if (curloc[1] > maxloc[3]) loc[1] = maxloc[3];
	}
	
	TransparentBlt(hDC, 100 + loc[0], 150 + loc[1], 100, 100, manDC, mv * 100, curmodeid * 100, 100, 100, RGB(255,255,255));
	
	if ((curmode & SQUAT) == SQUAT &&
		mv == 3)
	{
		mv--;
	}
	else mv++;
	
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
//			printf("down - %x\n", (unsigned int)wParam);
//			fflush(stdout);
			
			if (GetAction(wParam, &action))
			{
				if (GetModeID(nextmode | action, &modeid))
				{
					nextmode |= action;
					nextmodeid = modeid;
					
					if (curloc[1] == 0	&&
						curmodeid != nextmodeid)
					{
						curmode = mf[nextmodeid].mode;
						curmodeid = nextmodeid;
						mv = 0;
					}
				}
			}
			
			break;
		case WM_KEYUP:
//			printf("up - %x\n", (unsigned int)wParam);
//			fflush(stdout);
			
			if (GetAction(wParam, &action))
			{
				if (GetModeID(nextmode & (~action), &modeid))
				{
					nextmode &= ~action;
					nextmodeid = modeid;
					
					if (curloc[1] == 0	&&
						curmodeid != nextmodeid)
					{
						curmode = mf[nextmodeid].mode;
						curmodeid = nextmodeid;
						mv = 0;
					}
					else
					if (action == SQUAT	&&
						(curmode ^ nextmode) & SQUAT)
					{
						if (mv == 1)
						{
							curloc[1] += shift[curmodeid][mv].y;
						}
						mv = 4;
					}
				}
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