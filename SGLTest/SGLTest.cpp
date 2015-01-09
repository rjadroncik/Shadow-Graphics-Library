#include "StdAfx.h"
#include "resource.h"
#include "SGLTest.h"
#include "SGL.h"

#include <SCFMathematics.h>
#include <SCFTimer.h>
#include <SCFImaging.h>

using namespace ShadowGL;
using namespace SCFMathematics;
using namespace SCFTimer;
using namespace SCFImaging;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE	hInst;							

bool		Animate = TRUE;
bool		Focused = TRUE;
bool		Minimized = FALSE;
bool		ShowPlane = FALSE;
bool		ShowBox = TRUE;
bool		WasAnimating = FALSE;
bool		MouseLook = FALSE;

HWND		hWindow = NULL;

Float4	LightPos;
Float4	Diffuse;

Int			MoveMode = 0;

Float2	Angle;
Float2	Angle_Iterator;

Float	Scale = 1;
Double	Aspect = 1;

Float3	PlanePos;
Float3	CubePos01;
Float3	CubePos02;
Float3	CameraPos;
Float3	CameraAngle;

POINT		MousePos;
POINT		OriginalMousePos;

Int         TexName[16];

RECT		DrawRect;

//Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		Controls(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK		FogOptions(HWND, UINT, WPARAM, LPARAM);


void				DrawScene(HDC hDC);
void				DrawCube();

HRC					hRC = NULL;

BOOL				Quit = FALSE;

#define KEYDOWN(name, key) (((name)[(key)] & 0x80) != 0) 

BYTE KeysPressedLast[256] = { 0 };
BYTE KeysPressed[256]     = { 0 };

bool ButtonPressed(_IN UCHAR ucButton) { return (!KEYDOWN(KeysPressedLast, ucButton) &&  KEYDOWN(KeysPressed, ucButton)); } 

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg; msg.wParam = 0;

	//Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) { return FALSE; }

	ResetTime02();

	while (!Quit)  
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))  
		{
			//if ((msg.message == WM_KEYDOWN) ||
			//	(msg.message == WM_KEYUP) ||
			//	(msg.message == WM_CHAR))
			//{
			//	continue;
			//}

			if (msg.message == WM_QUIT) { Quit = TRUE; }
			else  
			{
				TranslateMessage(&msg);  
				DispatchMessage(&msg);   
			}
		}
		else
		{
			if (GetTime02() < 0.02) { continue; }
			else                    { ResetTime02(); }

			if (!Minimized & Focused)
			{
				memcpy(KeysPressedLast, KeysPressed, 256);
				GetKeyboardState(KeysPressed);

				if (MouseLook)
				{
					GetCursorPos(&MousePos); 

					RECT l_Rect;
					GetWindowRect(hWindow, &l_Rect);

					CameraAngle[1] -= Float(MousePos.x - (l_Rect.left + (l_Rect.right  - l_Rect.left) / 2)) / 10.0f;
					CameraAngle[0] -= Float(MousePos.y - (l_Rect.top  + (l_Rect.bottom - l_Rect.top)  / 2)) / 10.0f;

					SetCursorPos(l_Rect.left + (l_Rect.right - l_Rect.left) / 2, l_Rect.top + (l_Rect.bottom - l_Rect.top) / 2);
				}

				if (GetForegroundWindow() != hWindow)
				{
					Focused = FALSE;
					WasAnimating = Animate;
					Animate = FALSE;
				}
				else
				{
					switch (MoveMode)
					{
					case 1:
						{
							if (GetAsyncKeyState(VK_UP)    & 0x8000) { LightPos[1] += 10; }
							if (GetAsyncKeyState(VK_DOWN)  & 0x8000) { LightPos[1] -= 10; }
							if (GetAsyncKeyState(VK_LEFT)  & 0x8000) { LightPos[0] -= 10; }
							if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { LightPos[0] += 10; }
							break;
						}
					case 2:
						{
							if (GetAsyncKeyState(VK_UP)    & 0x8000) { CubePos01[1] += 10; }
							if (GetAsyncKeyState(VK_DOWN)  & 0x8000) { CubePos01[1] -= 10; }
							if (GetAsyncKeyState(VK_LEFT)  & 0x8000) { CubePos01[0] -= 10; }
							if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { CubePos01[0] += 10; }
							break;
						}
					case 3:
						{
							if (GetAsyncKeyState(VK_UP)    & 0x8000) { CubePos02[1] += 10; }
							if (GetAsyncKeyState(VK_DOWN)  & 0x8000) { CubePos02[1] -= 10; }
							if (GetAsyncKeyState(VK_LEFT)  & 0x8000) { CubePos02[0] -= 10; }
							if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { CubePos02[0] += 10; }
							break;
						}
					case 4:
						{
							if (GetAsyncKeyState(VK_UP)    & 0x8000) { PlanePos[1] += 10; }
							if (GetAsyncKeyState(VK_DOWN)  & 0x8000) { PlanePos[1] -= 10; }
							if (GetAsyncKeyState(VK_LEFT)  & 0x8000) { PlanePos[0] -= 10; }
							if (GetAsyncKeyState(VK_RIGHT) & 0x8000) { PlanePos[0] += 10; }
							break;
						}
					case 0:
						{
							if (GetAsyncKeyState(VK_UP) & 0x8000)
							{ 
								Float3 l_MoveVector = { 0, 0, -1 };

								RotateVector3(l_MoveVector, l_MoveVector, CameraAngle);
								ScaleVector3(l_MoveVector, l_MoveVector, 10);
								AddVectors3(CameraPos, CameraPos, l_MoveVector);
							}
							if (GetAsyncKeyState(VK_DOWN) & 0x8000)
							{ 
								Float3 l_MoveVector = { 0, 0, -1 };

								RotateVector3(l_MoveVector, l_MoveVector, CameraAngle);
								ScaleVector3(l_MoveVector, l_MoveVector, -10);
								AddVectors3(CameraPos, CameraPos, l_MoveVector);
							}
							if (GetAsyncKeyState(VK_LEFT) & 0x8000)
							{ 
								Float3 l_MoveVector = { 1, 0, 0 };

								RotateVector3(l_MoveVector, l_MoveVector, CameraAngle);
								ScaleVector3(l_MoveVector, l_MoveVector, -10);
								AddVectors3(CameraPos, CameraPos, l_MoveVector);
							}
							if (GetAsyncKeyState(VK_RIGHT) & 0x8000) 
							{
								Float3 l_MoveVector = { 1, 0, 0 };

								RotateVector3(l_MoveVector, l_MoveVector, CameraAngle);
								ScaleVector3(l_MoveVector, l_MoveVector, 10);
								AddVectors3(CameraPos, CameraPos, l_MoveVector);
							}
							break;
						}
					}


					if (ButtonPressed(VK_ESCAPE)) { DestroyWindow(hWindow); return 0; }

					if (ButtonPressed(VK_TAB)) { MoveMode++; if (MoveMode > 4) { MoveMode = 0; } }

					if (ButtonPressed(VK_OEM_3))
					{
						MouseLook = !MouseLook;

						if (MouseLook)
						{ 
							GetCursorPos(&OriginalMousePos); 

							SetCapture(hWindow);

							RECT l_Rect;
							GetWindowRect(hWindow, &l_Rect);
							SetCursorPos(l_Rect.left + (l_Rect.right - l_Rect.left) / 2, l_Rect.top + (l_Rect.bottom - l_Rect.top) / 2);
							ShowCursor(FALSE);
						}
						else
						{ 
							ReleaseCapture(); 			
							SetCursorPos(OriginalMousePos.x, OriginalMousePos.y); 
							ShowCursor(TRUE); 
						}
					}

					if (ButtonPressed(VK_SPACE)) { Animate   = !Animate; }
					if (ButtonPressed(0x50))     { ShowPlane = !ShowPlane; }
					if (ButtonPressed(0x42))     { ShowBox   = !ShowBox; }

					if (ButtonPressed(VK_ADD))      { Angle_Iterator[1] += 0.1f; }
					if (ButtonPressed(VK_SUBTRACT)) { Angle_Iterator[1] -= 0.1f; }

					if (ButtonPressed(0x46)) { if (IsEnabled(SGL_FOG))        { Disable(SGL_FOG); }        else { Enable(SGL_FOG); } }
					if (ButtonPressed(0x54)) { if (IsEnabled(SGL_TEXTURE_2D)) { Disable(SGL_TEXTURE_2D); } else { Enable(SGL_TEXTURE_2D); } }
					if (ButtonPressed(0x4C)) { if (IsEnabled(SGL_LIGHTING))   { Disable(SGL_LIGHTING); }   else { Enable(SGL_LIGHTING); } }
					if (ButtonPressed(0x53)) { Int Result; GetIntegerv(SGL_SHADE_MODEL, &Result); if (Result == SGL_FLAT) { ShadeModel(SGL_SMOOTH); } else { ShadeModel(SGL_FLAT); } }
				}

				if (Animate)
				{
					Angle[0] += Angle_Iterator[0];
					Angle[1] += Angle_Iterator[1];
				}

				RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
			}
		}
	}

	DeleteContext(hRC);

	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= TEXT("ShadowGLTestSession");
	wcex.hIconSm		= NULL;//LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_TEST);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	MyRegisterClass(hInstance);

	hWindow = CreateWindowEx(NULL, TEXT("ShadowGLTestSession"), TEXT("ShadowGL Test Session"), WS_OVERLAPPEDWINDOW | WS_SYSMENU, 100, 100, 640, 500, NULL, NULL, hInstance, NULL);
	if (!hWindow) { return FALSE; }
		
	TimerInitialize();

	HDC hDC = GetDC(hWindow);
	hRC = CreateContext(hDC);
	MakeCurrent(hDC, hRC);

	GetClientRect(hWindow, &DrawRect);
	Viewport(0, 0, DrawRect.right, DrawRect.bottom);

	Aspect = (Double)DrawRect.right / (Double)DrawRect.bottom;

	SetVector4(LightPos, 250, 250, 100, 1); 

	LightModeli(SGL_LIGHT_MODEL_LOCAL_VIEWER, TRUE);

	float tmp[4];
	
	tmp[0] = 0.1f;  
	tmp[1] = 0.1f;  
	tmp[2] = 0.1f;  
	tmp[3] = 0.8f;  
	LightModelfv(SGL_LIGHT_MODEL_AMBIENT, &tmp[0]);

	Enable(SGL_LIGHT0);
	Lightfv(SGL_LIGHT0, SGL_AMBIENT, &tmp[0]);

	tmp[0] = 1.0f;  
	tmp[1] = 1.0f;  
	tmp[2] = 1.0f;  
	tmp[3] = 1.0f; 
	Lightfv(SGL_LIGHT0, SGL_SPECULAR, &tmp[0]);

	tmp[0] = 1.0f;  
	tmp[1] = 1.0f;  
	tmp[2] = 1.0f;  
	tmp[3] = 1.0f;  
	Lightfv(SGL_LIGHT0, SGL_DIFFUSE, &tmp[0]);

	Enable(SGL_LIGHTING);

	ClearDepth(1);
	ClearColor(0.5, 0.5, 0.5, 0);

	Enable(SGL_FOG);

	tmp[0] = 0.5f;  
	tmp[1] = 0.5f;  
	tmp[2] = 0.5f;  
	tmp[3] = 1.0f;  

	ZeroVector2(Angle);
	SetVector2(Angle_Iterator, 1, 1.5f);
	
	SetVector3(PlanePos,    0,    0, -100);
	SetVector3(CubePos01,  100,  50,  -50);
	SetVector3(CubePos02, -100, -50,  -50);

	SetVector3(CameraPos,   0, 0, 600);
	SetVector3(CameraAngle, 0, 0,   0);

	ShadeModel(SGL_SMOOTH);

	Fogfv(SGL_FOG_COLOR, &tmp[0]);
	Fogf(SGL_FOG_DENSITY, 1);
	Fogf(SGL_FOG_MODE, SGL_LINEAR);
	Fogf(SGL_FOG_START, 200);
	Fogf(SGL_FOG_END, 2000);

	tmp[0] = 1.0f;  
	tmp[1] = 1.0f;  
	tmp[2] = 1.0f;  
	tmp[3] = 1.0f;  
	Materialfv(SGL_FRONT, SGL_SPECULAR, &tmp[0]);

	Float	Float01 = 10;
	Materialfv(SGL_FRONT, SGL_SHININESS, &Float01);

	tmp[0] = 1.0f;  
	tmp[1] = 0.0f;  
	tmp[2] = 0.0f;  
	tmp[3] = 1.0f;  
	Materialfv(SGL_FRONT, SGL_DIFFUSE, &tmp[0]);

/*	tmp[0] = 1.0f;  
	tmp[1] = 0.0f;  
	tmp[2] = 0.0f;  
	tmp[3] = 1.0f;  
	Materialfv(SGL_FRONT, SGL_EMISSION, &tmp[0]);*/

	GenTextures(16, &TexName[0]);

	CImage* pPicture = CImage::Load(TEXT("Data\\AMD 64.jpg"), NULL);
	if (pPicture)
	{
		switch (pPicture->Channels())
		{
			case 3:
			{
				BindTexture(SGL_TEXTURE_2D, TexName[0]);
				TexImage2D(SGL_TEXTURE_2D, 0, pPicture->Channels(), pPicture->Width(), pPicture->Height(), 0, SGL_RGB, SGL_UNSIGNED_BYTE, pPicture->Data());
				break;
			}
			case 4:
			{
				BindTexture(SGL_TEXTURE_2D, TexName[0]);
				TexImage2D(SGL_TEXTURE_2D, 0, pPicture->Channels(), pPicture->Width(), pPicture->Height(), 0, SGL_RGBA, SGL_UNSIGNED_BYTE, pPicture->Data());
				break;
			}
		}
		delete pPicture;
	}

	pPicture = CImage::Load(TEXT("Data\\NVidia.jpg"), NULL);
	if (pPicture)
	{
		switch (pPicture->Channels())
		{
		case 3:
			{
				BindTexture(SGL_TEXTURE_2D, TexName[1]);
				TexImage2D(SGL_TEXTURE_2D, 0, pPicture->Channels(), pPicture->Width(), pPicture->Height(), 0, SGL_RGB, SGL_UNSIGNED_BYTE, pPicture->Data());
				break;
			}
		case 4:
			{
				BindTexture(SGL_TEXTURE_2D, TexName[1]);
				TexImage2D(SGL_TEXTURE_2D, 0, pPicture->Channels(), pPicture->Width(), pPicture->Height(), 0, SGL_RGBA, SGL_UNSIGNED_BYTE, pPicture->Data());
				break;
			}
		}
		delete pPicture;
	}
		
	Enable(SGL_TEXTURE_2D);  

//	DeleteTextures(16, &TexName[0]);

	ShowWindow(hWindow, nCmdShow);
	UpdateWindow(hWindow);

	ReleaseDC(hWindow, hDC);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) 
	{
	case WM_MOVE:
		{
			Int2 MaxViewport;
			GetIntegerv(SGL_MAX_VIEWPORT_DIMS, MaxViewport);

			GetClientRect(hWnd, &DrawRect);

			DrawRect.right  = __min(MaxViewport[0], DrawRect.right  - DrawRect.left);
			DrawRect.bottom = __min(MaxViewport[1], DrawRect.bottom - DrawRect.top);

			Viewport(0, 0, DrawRect.right, DrawRect.bottom);

			Aspect = (Double)DrawRect.right / (Double)DrawRect.bottom;
			break;
		}

	case WM_SIZE:
		{
			Int2 MaxViewport;
			GetIntegerv(SGL_MAX_VIEWPORT_DIMS, MaxViewport);

			GetClientRect(hWnd, &DrawRect);

			DrawRect.right  = __min(MaxViewport[0], DrawRect.right  - DrawRect.left);
			DrawRect.bottom = __min(MaxViewport[1], DrawRect.bottom - DrawRect.top);

			Viewport(0, 0, DrawRect.right, DrawRect.bottom);

			Aspect = (Double)DrawRect.right / (Double)DrawRect.bottom;
			break;
		}

	case WM_COMMAND:
		{
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 

			switch (wmId)
			{
			case IDM_ABOUT:
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)About);
					break;
				}
				
			case IDM_CONTROLS:
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_CONTROLS), hWnd, (DLGPROC)Controls);
					break;
				}
				
			case IDM_OPTIONS_FOG:
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_OPTIONS_FOG), hWnd, (DLGPROC)FogOptions);
					break;
				}

			case IDM_EXIT:
				{
					DestroyWindow(hWnd);
					break;
				}
			default: { return DefWindowProc(hWnd, message, wParam, lParam); }
			}
			
			return 0;
		}
		 
	case WM_SETFOCUS:
		{
			if (WasAnimating) { Animate = TRUE; }
			Focused = TRUE;
			SetFocus(NULL);
			return 0;
		}

	case WM_KILLFOCUS:
		{
			return 0;
		}
		
	case WM_DESTROY:
		{
			DeleteContext(hRC);
			hRC = NULL;

			PostQuitMessage(0);
			break;
		}

	case WM_SYSCOMMAND:
		{
			if (wParam == SC_MINIMIZE)
			{
				Minimized = TRUE;
			}

			if (wParam == SC_RESTORE)
			{
				Minimized = FALSE;
			}
		}
	case WM_PAINT:
		{
			static PAINTSTRUCT ps;

			HDC hdc = BeginPaint(hWnd, &ps);

			DrawScene(hdc);

			EndPaint(hWnd, &ps);

			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	default: { return DefWindowProc(hWnd, message, wParam, lParam); }
	}
	return 0;
}

//Message handler for about box.
BOOL CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			const unsigned char	*Version;
			Version = GetString(SGL_VERSION);

			SetDlgItemText(hDlg, IDC_VERSION, CString((LPSTR)Version).Value());

			return TRUE;
		}

	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
		
			break;
		}
	}
    return FALSE;
}

//Message handler for Controls box.
BOOL CALLBACK Controls(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
		
			break;
		}
	}
    return FALSE;
}

//Message handler for about box.
BOOL CALLBACK FogOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			Float4 Float401;
			Float	Float01;
			Int		Int01;

			GetFloatv(SGL_FOG_COLOR, &Float401[0]);
			SetDlgItemInt(hDlg, IDC_COLOR_R, (UINT)(255 * Float401[0]), FALSE);
			SetDlgItemInt(hDlg, IDC_COLOR_G, (UINT)(255 * Float401[1]), FALSE);
			SetDlgItemInt(hDlg, IDC_COLOR_B, (UINT)(255 * Float401[2]), FALSE);

			GetFloatv(SGL_FOG_DENSITY, &Float01);
			SetDlgItemInt(hDlg, IDC_FOG_DENSITY, (UINT)(Float01 * 100), FALSE);

			GetFloatv(SGL_FOG_START, &Float01);
			SetDlgItemInt(hDlg, IDC_FOG_START, (UINT)Float01, FALSE);

			GetFloatv(SGL_FOG_END, &Float01);
			SetDlgItemInt(hDlg, IDC_FOG_END, (UINT)Float01, FALSE);

			GetIntegerv(SGL_FOG_MODE, &Int01);

			if (Int01 == SGL_LINEAR)
			{
				CheckRadioButton(hDlg, IDC_LINEAR, IDC_EXP2, IDC_LINEAR);
			}
		
			if (Int01 == SGL_EXP)
			{
				CheckRadioButton(hDlg, IDC_LINEAR, IDC_EXP2, IDC_EXP);
			}
			
			if (Int01 == SGL_EXP2)
			{
				CheckRadioButton(hDlg, IDC_LINEAR, IDC_EXP2, IDC_EXP2);
			}

			return TRUE;
		}

	case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				{
					break;
				}
			}

			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
		
			break;
		}

	case WM_DESTROY:
		{
			Float4 Float401;
			Float	Float01;

			Float401[0] = GetDlgItemInt(hDlg, IDC_COLOR_R, NULL, FALSE) / 255.0f;
			Float401[1] = GetDlgItemInt(hDlg, IDC_COLOR_G, NULL, FALSE) / 255.0f;
			Float401[2] = GetDlgItemInt(hDlg, IDC_COLOR_B, NULL, FALSE) / 255.0f;
			Float401[3] = 1;
			Fogfv(SGL_FOG_COLOR, &Float401[0]);

			Float01 = GetDlgItemInt(hDlg, IDC_FOG_DENSITY, NULL, FALSE) / 100.0f;
			Fogf(SGL_FOG_DENSITY, Float01);

			Float01 = (Float)GetDlgItemInt(hDlg, IDC_FOG_START, NULL, FALSE);
			Fogf(SGL_FOG_START, Float01 );

			Float01 = (Float)GetDlgItemInt(hDlg, IDC_FOG_END, NULL, FALSE);
			Fogf(SGL_FOG_END, Float01);

			if (IsDlgButtonChecked(hDlg, IDC_LINEAR) == BST_CHECKED)
			{
				Fogi(SGL_FOG_MODE, SGL_LINEAR);
			}

			if (IsDlgButtonChecked(hDlg, IDC_EXP) == BST_CHECKED)
			{
				Fogi(SGL_FOG_MODE, SGL_EXP);
			}

			if (IsDlgButtonChecked(hDlg, IDC_EXP2) == BST_CHECKED)
			{
				Fogi(SGL_FOG_MODE, SGL_EXP2);
			}

			break;
		}

	}
    return FALSE;
}

void DrawScene(HDC hDC)
{
	if (Minimized) { return; }

	ResetTime01();

	Clear(SGL_COLOR_BUFFER_BIT | SGL_DEPTH_BUFFER_BIT);

	MatrixMode(SGL_PROJECTION);
	LoadIdentity();

//	Frustum(-1, 1, -1, 1, 10, 10000);
	Perspective(45, Aspect, 10, 2000);

	MatrixMode(SGL_MODELVIEW);
	LoadIdentity();

	Rotatef(-CameraAngle[0], 1, 0, 0);
	Rotatef(-CameraAngle[1], 0, 1, 0);
	//Rotatef(-CameraAngle[2], 0, 0, 1);

	Translatef(-CameraPos[0], -CameraPos[1], -CameraPos[2]);

	Lightfv(SGL_LIGHT0, SGL_POSITION, &LightPos[0]);

	PushMatrix();
	{
		Disable(SGL_TEXTURE_2D);
		Disable(SGL_LIGHTING);

		Translatef(LightPos[0], LightPos[1], LightPos[2]);

		Scalef(0.1f, 0.1f, 0.1f);

		SetVector4(Diffuse, 1.0f, 0.0f, 0.0f, 1);
		Materialfv(SGL_FRONT, SGL_DIFFUSE, &Diffuse[0]);

		DrawCube();
	
		Enable(SGL_LIGHTING);
		Enable(SGL_TEXTURE_2D);
	}
	PopMatrix();

	if (ShowBox)
	{
		PushMatrix();
		{
			BindTexture(SGL_TEXTURE_2D, TexName[0]);

			Translatef(CubePos01[0], CubePos01[1], CubePos01[2]);

			Rotatef(Angle[0], 1, 0, 0);
			Rotatef(Angle[1], 0, 1, 0);

			Scalef(Scale, Scale, Scale);

			SetVector4(Diffuse, 0.7f, 0.7f, 0.6f, 1);
			Materialfv(SGL_FRONT, SGL_DIFFUSE, &Diffuse[0]);

			DrawCube();
		}
		PopMatrix();

		PushMatrix();
		{
			BindTexture(SGL_TEXTURE_2D, TexName[1]);

			Translatef(CubePos02[0], CubePos02[1], CubePos02[2]);

			Rotatef(30.0f, 1, 0, 0);
			Rotatef(30.0f, 0, 1, 0);
			Rotatef(30.0f, 0, 0, 1);

			Rotatef(Angle[0], 1, 0, 0);
			Rotatef(Angle[1], 0, 1, 0);

			Scalef(Scale, Scale, Scale);

			SetVector4(Diffuse, 0.7f, 0.7f, 0.6f, 1);
			Materialfv(SGL_FRONT, SGL_DIFFUSE, &Diffuse[0]);

			DrawCube();
		}
		PopMatrix();
	}

	if (ShowPlane)
	{
		SetVector4(Diffuse, 1, 1, 1, 1);
		Materialfv(SGL_FRONT, SGL_DIFFUSE, &Diffuse[0]);

		Translatef(PlanePos[0], PlanePos[1], PlanePos[2]);

		Begin(SGL_TRIANGLES);
		{
			Normal3f(0.0f, 0.0f, 1.0f);

			Color3f(1.0f, 0.0f, 0.0f); TexCoord2f(0.0f, 0.0f); Vertex3f(-200.0f, -200.0f, 0.0f);
			Color3f(0.0f, 1.0f, 0.0f); TexCoord2f(1.0f, 0.0f); Vertex3f(200.0f, -200.0f, 0.0f);
			Color3f(0.0f, 0.0f, 1.0f); TexCoord2f(1.0f, 1.0f); Vertex3f(200.0f,  200.0f, 0.0f);

			Color3f(1.0f, 0.0f, 0.0f); TexCoord2f(0.0f, 0.0f); Vertex3f(-200.0f, -200.0f, 0.0f);
			Color3f(0.0f, 0.0f, 1.0f); TexCoord2f(1.0f, 1.0f); Vertex3f(200.0f, 200.0f, 0.0f);
			Color3f(1.0f, 1.0f, 0.0f); TexCoord2f(0.0f, 1.0f); Vertex3f(-200.0f, 200.0f, 0.0f);
		}
		End();
	}

	static float   s_fTotalTime	= 0;
	static int	   s_iRenderCount = 0;
	static CString s_Title;

	s_fTotalTime += GetTime01();
	s_iRenderCount++;

	if (s_iRenderCount == 50)
	{
		s_Title = STRING("ShadowGL Test Session, FPS: ");
		s_Title += CInt((int)(50.f / s_fTotalTime)).ToString(); 

		SetWindowText(hWindow, s_Title.Value());

		s_fTotalTime	= 0;
		s_iRenderCount	= 0;
	}

	ShadowGL::SwapBuffers(hDC);
}

void DrawCube()
{
	Begin(SGL_TRIANGLES);
	{
		//Top side
		Normal3f(0.0f, 1.0f, 0.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f, 100.0f, 100.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, 100.0f,-100.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 1.0f); Vertex3f(-100.0f, 100.0f, 100.0f);

		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(	100.0f, 100.0f, 100.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(1.0f, 0.0f); Vertex3f(	100.0f, 100.0f,-100.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, 100.0f,-100.0f);

		//Bottom side
		Normal3f(0.0f, -1.0f, 0.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f, -100.0f, 100.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 1.0f); Vertex3f(-100.0f, -100.0f, 100.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, -100.0f,-100.0f);

		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(	100.0f, -100.0f, 100.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, -100.0f,-100.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 0.0f); Vertex3f(	100.0f, -100.0f,-100.0f);

		//Front side
		Normal3f(0.0f, 0.0f, 1.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, -100.0f, 100.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 0.0f); Vertex3f(100.0f, -100.0f, 100.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f,  100.0f, 100.0f);

		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, -100.0f, 100.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f, 100.0f, 100.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 1.0f); Vertex3f(-100.0f, 100.0f, 100.0f);

		//Back side
		Normal3f(0.0f, 0.0f, -1.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f,  100.0f, -100.0f);
		Color3f(0.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 0.0f); Vertex3f(100.0f, -100.0f, -100.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, -100.0f, -100.0f);

		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f, 100.0f, -100.0f);
		Color3f(1.0f, 0.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f, -100.0f, -100.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 1.0f); Vertex3f(-100.0f, 100.0f, -100.0f);

		//Right side
		Normal3f(1.0f, 0.0f, 0.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(100.0f,-100.0f,-100.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 0.0f); Vertex3f(100.0f, 100.0f,-100.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f, 100.0f, 100.0f);

		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(100.0f,-100.0f,-100.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(100.0f, 100.0f, 100.0f);
		Color3f(0.0f, 0.0f, 1.0f);	TexCoord2f(0.0f, 1.0f); Vertex3f(100.0f,-100.0f, 100.0f);

		//Left side
		Normal3f(-1.0f, 0.0f, 0.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f,-100.0f,-100.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(-100.0f, 100.0f, 100.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 0.0f); Vertex3f(-100.0f, 100.0f,-100.0f);

		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 0.0f); Vertex3f(-100.0f,-100.0f,-100.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(0.0f, 1.0f); Vertex3f(-100.0f,-100.0f, 100.0f);
		Color3f(1.0f, 1.0f, 0.0f);	TexCoord2f(1.0f, 1.0f); Vertex3f(-100.0f, 100.0f, 100.0f);
	}
	End();
}