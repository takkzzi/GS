#include "PCH.h"
#include "Stream.h"

using namespace Core;

Stream::Stream()
{
	mBufferPointer	= NULL;
	mLength			= 0;
}

Stream::~Stream()
{
}

BOOL Stream::SetBuffer(BYTE *buffer)
{
	if (!buffer)
		return FALSE;

	mBufferPointer	= buffer;
	mLength			= 0;
	return TRUE;
}

BOOL Stream::SetBuffer()
{
	if (!mBufferPointer)
		return FALSE;

	mLength			= 0;
	return TRUE;
}

BOOL Stream::ReadInt32(INT *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(INT));
	mLength += sizeof(INT);
	return TRUE;
}

BOOL Stream::ReadDWORD(DWORD *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(DWORD));
	mLength += sizeof(DWORD);
	return TRUE;
}

BOOL Stream::ReadDWORD_PTR(DWORD_PTR *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(DWORD_PTR));
	mLength += sizeof(DWORD_PTR);
	return TRUE;
}

BOOL Stream::ReadByte(BYTE *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(BYTE));
	mLength += sizeof(BYTE);
	return TRUE;
}

BOOL Stream::ReadBytes(BYTE *data, DWORD length)
{
	memcpy(data, mBufferPointer + mLength, length);
	mLength += length;
	return TRUE;
}

BOOL Stream::ReadFloat(FLOAT *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(FLOAT));
	mLength += sizeof(FLOAT);
	return TRUE;
}

BOOL Stream::ReadInt64(INT64 *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(INT64));
	mLength += sizeof(INT64);
	return TRUE;
}

BOOL Stream::ReadSHORT(SHORT *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(SHORT));
	mLength += sizeof(SHORT);
	return TRUE;
}

BOOL Stream::ReadUSHORT(USHORT *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(USHORT));
	mLength += sizeof(USHORT);
	return TRUE;
}

BOOL Stream::ReadBOOL(BOOL *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(BOOL));
	mLength += sizeof(BOOL);
	return TRUE;
}

BOOL Stream::ReadWCHAR(WCHAR *data)
{
	memcpy(data, mBufferPointer + mLength, sizeof(WCHAR));
	mLength += sizeof(WCHAR);
	return TRUE;
}

BOOL Stream::ReadWCHARs(LPWSTR data, DWORD length)
{
	memcpy(data, mBufferPointer + mLength, length * sizeof(WCHAR));
	mLength += length * sizeof(WCHAR);
	return TRUE;
}

BOOL Stream::WriteInt32(INT data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(INT));
	mLength += sizeof(INT);
	return TRUE;
}

BOOL Stream::WriteDWORD(DWORD data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(DWORD));
	mLength += sizeof(DWORD);
	return TRUE;
}

BOOL Stream::WriteDWORD_PTR(DWORD_PTR data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(DWORD_PTR));
	mLength += sizeof(DWORD_PTR);
	return TRUE;
}

BOOL Stream::WriteByte(BYTE data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(BYTE));
	mLength += sizeof(BYTE);
	return TRUE;
}

BOOL Stream::WriteBytes(BYTE *data, DWORD length)
{
	memcpy(mBufferPointer + mLength, data, length);
	mLength += length;
	return TRUE;
}

BOOL Stream::WriteFloat(FLOAT data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(FLOAT));
	mLength += sizeof(FLOAT);
	return TRUE;
}

BOOL Stream::WriteInt64(INT64 data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(INT64));
	mLength += sizeof(INT64);
	return TRUE;
}

BOOL Stream::WriteSHORT(SHORT data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(SHORT));
	mLength += sizeof(SHORT);
	return TRUE;
}

BOOL Stream::WriteUSHORT(USHORT data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(USHORT));
	mLength += sizeof(USHORT);
	return TRUE;
}

BOOL Stream::WriteBOOL(BOOL data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(BOOL));
	mLength += sizeof(BOOL);
	return TRUE;
}

BOOL Stream::WriteWCHAR(WCHAR data)
{
	memcpy(mBufferPointer + mLength, &data, sizeof(WCHAR));
	mLength += sizeof(WCHAR);
	return TRUE;
}

BOOL Stream::WriteWCHARs(LPCWSTR data, DWORD length)
{
	memcpy(mBufferPointer + mLength, data, length * sizeof(WCHAR));
	mLength += length * sizeof(WCHAR);
	return TRUE;
}
