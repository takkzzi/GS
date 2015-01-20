// TestServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TestServer.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);




const int port			= 42006;
const int sessionCount	= 1;
const int sessionLimit	= 1000;

class EchoServer : public Thread 
{
public :

	EchoServer() : Thread()
	{
		mIocp = new Networker(true, 500, sessionCount, sessionLimit, 1024, 19);

		mSendTime = Time::GetAppTime();
	};

	virtual ~EchoServer() 
	{
		SAFE_DELETE(mIocp);
	};

	virtual bool Begin(bool bSuspend=false)
	{
		mIocp->BeginListen(port, true);

		return __super::Begin(bSuspend);
	}

	virtual bool End()
	{
		/*
		//DisconnectAll
		for(int i = 0; i < mIocp->GetSessionCount(); ++i) {
			Session* se = mIocp->GetSession(i);
			se->Disconnect();
		}
		*/
		return __super::End();
	}

	virtual DWORD ThreadTick()
	{
		if ( IsState(THREAD_END) )
			return 0;

		bool bDataEcho = true;

		double currTime = Time::GetAppTime();
		if ( (currTime - mSendTime) > 3.0) {
			mSendTime = currTime;
		}

		if ( ! mIocp->IsThreadUpdatingSessions() )
			mIocp->UpdateSessions();

		ReadData(bDataEcho);
		return 1;
	}

	void ReadData(bool bEcho)
	{
		for(int i = 0; i < mIocp->GetSessionCount(); ++i) {
			Session* se = mIocp->GetSession(i);
			if ( se && se->IsState(SESSIONSTATE_CONNECTED) ) 
			{
				while( PacketBase* packet = se->ReadData() )	
				{	
					ASSERT( packet->mPacketSize == sizeof(AlphabetPacket) );

					AlphabetPacket* alpha = (AlphabetPacket*)packet;

					if ( bEcho ) 
						se->WriteData((char*)(packet), packet->mPacketSize);	//Echo

					se->ClearRecv(sizeof(AlphabetPacket));
				}
			}
		}
	}

	void SessionLog(Session* sess, char* msg, size_t size ) {
		char totalMsg[2048];

		memcpy(totalMsg, msg, size);
		totalMsg[size] = '\0';

		sprintf_s(totalMsg, "%s\n", totalMsg);
		//OutputDebugStringA(totalMsg);
	}

protected :
	Networker*					mIocp;
	std::vector<Session*>		mSessions;

	double						mSendTime;
};



int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TESTSERVER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTSERVER));

	CoreSystem::Init(_T("ServerLog"));
	NetworkSystem::Init();

	EchoServer* server = new EchoServer();
	server->Begin();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	server->End();
	SAFE_DELETE(server);

	NetworkSystem::Shutdown();
	CoreSystem::Shutdown();

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTSERVER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TESTSERVER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

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

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
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
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
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
