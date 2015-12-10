#include "stdafx.h"
#include "Client.h"
#include "GameCommon/GamePacketBase.h"

using namespace Game;


Client::Client(TcpSession* session)
	: mSession(session)
	, mPacketSequence(1)
	, mSentElapsedTime(0.0f)
{
}

Client::~Client()
{
}

bool Client::Connect(const CHAR* addr, const int port)
{
	bool bConn = mSession->Connect(addr, port);
	if (bConn)
		mSession->SetEventObject(this);

	return bConn;
}

void Client::Disconnect()
{
	mSession->Disconnect();
}

void Client::SendPacket()
{
	if (!mSession->IsConnected())
		return;

	static TCHAR* chatStr = _T("동해물과 백두산이 마르고 닳도록 하느님이 보우하사 우리 나라만세. 무궁화 삼천리 화려 강산 대한 사람 대한으로 길이 보전하세.");
	ChatMsg chatmsg(chatStr);

	chatmsg.mSequence = mPacketSequence;
	bool bSent = mSession->WriteToSend((char*)&chatmsg, chatmsg.mSize);
	if (bSent) {
		mPacketSequence++;
	}

	chatmsg.mSequence = mPacketSequence;
	bSent = mSession->WriteToSend((char*)&chatmsg, chatmsg.mSize);
	if (bSent) {
		mPacketSequence++;
	}

	AlphabetPacket alphabet;

	alphabet.mSequence = mPacketSequence;
	bSent = mSession->WriteToSend((char*)&alphabet, alphabet.mSize);
	if (bSent) {
		mPacketSequence++;
	}

	alphabet.mSequence = mPacketSequence;
	bSent = mSession->WriteToSend((char*)&alphabet, alphabet.mSize);
	if (bSent) {
		mPacketSequence++;
	}


}

void Client::RecvPacket()
{
	if (!mSession->IsConnected())
		return;

	while(GamePacketBase* packetData = (GamePacketBase*)GetPacket())
	{ 
		if (packetData->mType == (USHORT)PT_ChatMsg) {
			ChatMsg* chatMsg = (ChatMsg*)packetData;
			//Logger::LogDebugString(_T("(%d) %s"), mSession->GetId(), chatMsg->mChatData);
		}
		else if (packetData->mType == (USHORT)PT_Alphabet) {
			AlphabetPacket* alphabet = (AlphabetPacket*)packetData;
			//Logger::LogDebugString("(%d) %s", mSession->GetId(), alphabet->mData);
		}
		else {
			ASSERT(0);
		}

		ClientSimulator::msRecvedData += packetData->mSize;

		mSession->ClearRecvBuffer(packetData->mSize);
	}
}

void* Client::GetPacket()
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

/*
void Client::Update()
{
	if (!mSession->IsConnected())
		return;

	SendPacket();
	RecvPacket();
}
*/


void Client::OnDisconnect()
{
	Logger::LogDebugString("[%d] Disconnected !", mSession->GetId());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* gServerIP = "192.168.0.14";
const int	gServerPort = 4999;

const int	gClientCount = 1000;
const int	gBufferSize = 4096;


volatile UINT64 ClientSimulator::msRecvedData = 0;

ClientSimulator::ClientSimulator() : Thread()
{
	mIocp = new TcpNetworker(true, 3, gClientCount, gClientCount + 1, gBufferSize, gBufferSize);
	mClients.reserve(gClientCount);
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

	mPrevAppTime = Core::Time::GetAppTime();

	return __super::Begin(bSuspend);
}

DWORD ClientSimulator::ThreadTick()
{

	RunSimulation();
	Sleep(0);

	return 1;
}

void ClientSimulator::RunSimulation()
{

	double currentTime = Core::Time::GetAppTime();

	for (auto &c : mClients) {
		if (currentTime - mPrevAppTime >= 0.001)
			c->SendPacket();

		c->RecvPacket();
	}

	mPrevAppTime = currentTime;
}

void ClientSimulator::Update()
{
	static double prevAppTime = Core::Time::GetAppTime();

	double currAppTime = Core::Time::GetAppTime();
	if (currAppTime - prevAppTime >= 1.0)
	{
		float kbSize = (float)msRecvedData / 1024.0f;

		if (kbSize>0.0f)
			Logger::LogDebugString("Recieved Data : %g KB/S", kbSize);

		msRecvedData = 0;

		prevAppTime = currAppTime;
	}
}

bool ClientSimulator::End()
{
	return __super::End();
}



