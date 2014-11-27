#pragma once

#define MAX_BUFFER_LENGTH	4096

namespace Network
{
	enum IOKeyType 
	{
		IOKey_Listener,
		IOKey_Session
	};

	class IOKey 
	{
	public :
		IOKeyType			mType;
	};
}
