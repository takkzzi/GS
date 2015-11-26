#include "stdafx.h"
#include "Client.h"
#include "GameCommon/GamePacketBase.h"

using namespace Game;


Client::Client(TcpSession* session)
	: mSession(session)
{
}


Client::~Client()
{
}

bool Client::Connect(const CHAR* addr, const int port)
{
	return mSession->Connect(addr, port);
}

void Client::Disconnect()
{
	mSession->Disconnect();
}

void Client::SendPacket()
{
	static TCHAR* chatStr = _T("동해물과 백두산이 마르고 닳도록 하느님이 보우하사 우리 나라만세.\0 무궁화 삼천리 화려 강산 대한 사람 대한으로 길이 보전하세.");
	ChatMsg chatmsg(chatStr);

	mSession->WriteToSend((char*)&chatmsg, chatmsg.mSize);

}

void Client::RecvPacket()
{
}

void Client::Update()
{
	if (!mSession->IsConnected())
		return;

	SendPacket();
	RecvPacket();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* gServerIP = "127.0.0.1";//"192.168.0.14";
const int	gServerPort = 9500;


ClientSimulator::ClientSimulator(int connCount) : Thread()
{
	mIocp = new TcpNetworker(true, 3, connCount, connCount + 1, 256, 256);
};

ClientSimulator::~ClientSimulator()
{
	SAFE_DELETE(mIocp);
	SAFE_DELETE(mClient);
};

bool ClientSimulator::Begin(bool bSuspend)
{
	/*
	//ConnectAll
	for (int i = 0; i < mIocp->GetSessionCount(); ++i) {
		TcpSession* se = mIocp->GetSession(i);
		bool bConn = se->Connect(gServerIP, gServerPort);
		if (!bConn) {
		}
	}
	*/

	TcpSession* sess = mIocp->GetNewSession();
	mClient = new Client(sess);
	mClient->Connect(gServerIP, gServerPort);

	mLastSendTime = Core::Time::GetAppTime();

	return __super::Begin(bSuspend);
}

DWORD ClientSimulator::ThreadTick()
{

	double currTime = Time::GetAppTime();
	if (currTime - mLastSendTime > 0.2) {

		RunSimulation();

		mLastSendTime = currTime;
	};


	Sleep(0);

	return 1;
}

void ClientSimulator::RunSimulation()
{
	/*
	for (int i = 0; i < mIocp->GetSessionCount(); ++i) {
		TcpSession* se = mIocp->GetSession(i);
		if (se->IsConnected()) {
			if (Core::Math::RandRange(0, 1000) > 500)
				se->Disconnect();
		}
		else {
			if (Core::Math::RandRange(0, 1000) > 500) {
				bool bCon = se->Connect(gServerIP, gServerPort);
				if (!bCon) {
				}
			}
		}
	}
	*/

	mClient->Update();
}



bool ClientSimulator::End()
{
	return __super::End();
}



