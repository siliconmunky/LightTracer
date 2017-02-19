#include "BasicDefs.h"

#include "Game.h"

#include "Windows.h"
#include "Windowsx.h"
#include "WinDef.h"



HWND gHWnd = NULL;
int gWidth = 1280;
int gHeight = 720;
bool gVsync = false;
bool gFullScreen = false;



LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message )
	{
	case WM_PAINT:
		hdc = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}

bool InitWindow()
{
	WNDCLASSEX wc;

	// Get the instance of this application.
	HINSTANCE m_hinstance = GetModuleHandle(NULL);
	
	LPCWSTR m_applicationName;
	m_applicationName = L"RayCast";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	int pos_x, pos_y;
	DEVMODE dmScreenSettings;
	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	
	if(gFullScreen)
	{
		//Force to native res
		//gWidth = GetSystemMetrics(SM_CXSCREEN);
		//gHeight = GetSystemMetrics(SM_CYSCREEN);

		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)gWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)gHeight;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		pos_x = pos_y = 0;
	}
	else
	{

		// Place the window in the middle of the screen.
		pos_x = (GetSystemMetrics(SM_CXSCREEN) - gWidth)  / 2;
		pos_y = (GetSystemMetrics(SM_CYSCREEN) - gHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	gHWnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    pos_x, pos_y, gWidth, gHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(gHWnd, SW_SHOW);
	SetForegroundWindow(gHWnd);
	SetFocus(gHWnd);

	if( !gHWnd)
	{
		DWORD err = GetLastError();
		return false;
	}

	return true;
}





int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	if( !InitWindow())
	{
		exit(-1);
	}


	Game* game = new Game();
	


	static int mid_width = GetSystemMetrics(SM_CXSCREEN) / 2;
	static int mid_height = GetSystemMetrics(SM_CYSCREEN) / 2;


	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); //required for each thread to use fmod

	//init the mouse position so we don't spike on boot
	SetCursorPos(mid_width, mid_height);
	ShowCursor(false);

	bool done = false;
	while( !done )
	{
		Input::Instance->SetInputMotion(IX_MOUSE_X, 0);
		Input::Instance->SetInputMotion(IX_MOUSE_Y, 0);

		// Handle the windows messages.
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			// If windows signals to end the application then exit out.
			if(msg.message == WM_QUIT)
			{
				done = true;
			}

			switch (msg.message)
			{
				// Check if a key has been pressed on the keyboard.
				case WM_KEYDOWN:
				{
					Input::Instance->SetKeyState((IX_KEY)msg.wParam, true);
				}
				break;

				// Check if a key has been released on the keyboard.
				case WM_KEYUP:
				{
					Input::Instance->SetKeyState((IX_KEY)msg.wParam, false);
				}
				break;
			}
		}

		if (gHWnd == GetForegroundWindow())
		{
			POINT pos_m;
			GetCursorPos(&pos_m);
			int x_pos = pos_m.x;
			int y_pos = pos_m.y;

			SetCursorPos(mid_width, mid_height);

			float x_move = (float)(x_pos - mid_width);
			float y_move = (float)(y_pos - mid_height);

			Input::Instance->SetInputMotion(IX_MOUSE_X, x_move);
			Input::Instance->SetInputMotion(IX_MOUSE_Y, y_move);
		}

		game->GameLoop();
	}
	

	CoUninitialize(); //to finish using fmod
	
    return 0;
}
