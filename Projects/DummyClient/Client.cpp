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
	mSession->Disconnect();
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
	static TCHAR* chatStr = _T("동해물과 백두산이 마르고 닳도록 하느님이 보우하사 우리 나라만세. 무궁화 삼천리 화려 강산 대한 사람 대한으로 길이 .");
	ChatMsg chatmsg(chatStr);

	static bool bTest = true;

	//if (bTest) 
	{
		mSession->WriteToSend((char*)&chatmsg, chatmsg.mSize);
		mSession->WriteToSend((char*)&chatmsg, chatmsg.mSize);

		bTest = false;
	}

	AlphabetPacket alphabet;
	mSession->WriteToSend((char*)&alphabet, alphabet.mSize);
	mSession->WriteToSend((char*)&alphabet, alphabet.mSize);

}

void Client::RecvPacket()
{
	//mSession->ReadRecvBuffer()
	//ChatMsg* chatMsg = GetChatPacket();

	ChatMsg* chatMsg = (ChatMsg*)GetChatPacket();
	if (chatMsg) {

		LOG(_T("ChatTest"), _T("(%d) %s"), mSession->GetId(), chatMsg->mChatData);
		
		mSession->ClearRecvBuffer(chatMsg->mSize);
	}
}

void* Client::GetChatPacket()
{
	GamePacketBase* basePacket = NULL;
	char* baseData = mSession->ReadRecvBuffer(sizeof(GamePacketBase));
	if (baseData) {
		basePacket = (GamePacketBase*)baseData;
		char* entireData = mSession->ReadRecvBuffer(basePacket->mSize);
		if (entireData) {
			return basePacket;
		}
	}
	return NULL;
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

const int	gBufferSize = 1024;

ClientSimulator::ClientSimulator(int clientCount) : Thread()
{
	mIocp = new TcpNetworker(true, 3, clientCount, clientCount + 1, gBufferSize, gBufferSize);
	mClients.reserve(clientCount);
};

ClientSimulator::~ClientSimulator()
{
	SAFE_DELETE(mIocp);
	for (auto &c : mClients) {
		SAFE_DELETE(c);
	}
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

	for (int i = 0, n = mIocp->GetSessionCount(); i < n; ++i) {
		Client* newClient = new Client(mIocp->GetSession(i));
		mClients.push_back(newClient);
		newClient->Connect(gServerIP, gServerPort);
	}

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

	for (auto &c : mClients) {
		c->Update();
	}
}



bool ClientSimulator::End()
{
	return __super::End();
}



