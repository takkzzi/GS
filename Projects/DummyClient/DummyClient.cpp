// DummyClient.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DummyClient.h"

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

const char* gServerIP = "127.0.0.1";//"192.168.0.14";
const int	gServerPort = 42006;


class PingPongClient : public Thread 
{
public :

	PingPongClient(int connCount) : Thread()
	{
		mIocp = new Networker(true, 3, connCount, 100, 2048, 1024);

		ZeroMemory(mSendCounters, 5000*sizeof(UINT64));
		ZeroMemory(mRecvCounters, 5000*sizeof(UINT64));
	};

	virtual ~PingPongClient() 
	{
		SAFE_DELETE(mIocp);
	};

	virtual bool Begin(bool bSuspend=false)
	{
		//ConnectAll
		for(int i = 0; i < mIocp->GetSessionCount(); ++i) {
			Session* se = mIocp->GetSession(i);
			bool bConn = se->Connect(gServerIP, gServerPort);

			ASSERT(bConn);
		}

		mLastSendTime = Core::Time::GetAppTime();

		return __super::Begin(bSuspend);
	}

	virtual DWORD ThreadTick()
	{

		double currTime = Time::GetAppTime();
		if ( currTime - mLastSendTime > 1.0) {
			PushSend(10);
			mLastSendTime = currTime;
		};

		PopRecv();

		//mIocp->UpdateSessions();

		Sleep(10);

		return 1;
	}

	void PushSend(int pushCount)
	{
		for(int i = 0; i < mIocp->GetSessionCount(); ++i) {
			Session* se = mIocp->GetSession(i);
			if ( se && se->IsState(SESSIONSTATE_CONNECTED) ) {
				
				for (int j = 0 ; j < pushCount; ++j) {
					AlphabetPacket alpha;
					alpha.mPacketSize = sizeof(AlphabetPacket);
				
					bool bSend = se->WriteData((char*)&alpha, sizeof(AlphabetPacket));
					ASSERT(bSend);
				}
			}
		}

		
	}

	void PopRecv() 
	{
		for(int i = 0; i < mIocp->GetSessionCount(); ++i) {
			Session* se = mIocp->GetSession(i);
			if ( se && se->IsState(SESSIONSTATE_CONNECTED) ) {
				while(PacketBase* packet = se->ReadData())
				{
					ASSERT( packet->mPacketSize == sizeof(AlphabetPacket) );
					AlphabetPacket* alpha = (AlphabetPacket*)packet;
					//PrintDebugString("Recved : ", packet);
					se->ClearRecv(packet->mPacketSize);
				}
			}
		}
	}

	/*
	void PrintDebugString(char* msg, PacketBase* packet ) {
		char totalMsg[2048];

		sprintf_s(totalMsg, "%s (size:%d)\n", msg, packet->mPacketSize);
		OutputDebugStringA(msg);
	}
	*/


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


protected :
	Networker*					mIocp;
	std::vector<Session*>		mSessions;

	UINT64						mSendCounters[5000];
	UINT64						mRecvCounters[5000];

	double						mLastSendTime;
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
	LoadString(hInstance, IDC_DUMMYCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DUMMYCLIENT));


	CoreSystem::Init(_T("ClientLog"));
	NetworkSystem::Init();

	
	PingPongClient* sessTester[5] = { 0, 0, 0, 0, 0 };

	sessTester[0] = new PingPongClient(1);
	sessTester[0]->Begin(false);
	
	//sessTester[1] = new PingPongClient(10);
	//sessTester[1]->Begin(false);

	//sessTester[2] = new PingPongClient(10);
	//sessTester[2]->Begin(false);

	//sessTester[3] = new PingPongClient(1000);
	//sessTester[3]->Begin(false);

	//sessTester[4] = new PingPongClient(1000);
	//sessTester[4]->Begin(false);

	// Main message loop:
	double startTime = Time::GetAppTime();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			if( msg.message == WM_QUIT )
				break;

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			//sessTester[0]->Update();
		}
	}


	sessTester[0]->End();
	delete sessTester[0];

	/*
	sessTester[1]->End();
	delete sessTester[1];
	
	sessTester[2]->End();
	delete sessTester[2];

	sessTester[3]->End();
	delete sessTester[3];

	sessTester[4]->End();
	delete sessTester[4];
	*/

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DUMMYCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DUMMYCLIENT);
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
