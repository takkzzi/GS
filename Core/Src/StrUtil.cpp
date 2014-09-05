#include "PCH.h"
#include "StrUtil.h"
#include <cstring>
#include <cstdlib>

using namespace Core;

void StrUtil::Hex2Ascii(const unsigned char* hex, char* data, size_t srclen)
{
	data[0] = '\0';
	char szChunk[4];
	for (size_t i=0; i<srclen; ++i)
	{
		sprintf(szChunk, "%02X", hex[i]);
		strcat(data, szChunk);
	}
	//data[ srclen ] = '\0';
}

void StrUtil::Ascii2Hex(const char* data, unsigned char* hex, size_t len)
{
	for (size_t i=0; i<(len/2); ++i)
	{
		sscanf(data + (i*2), "%02x", &hex[i]);
	}
}

//FILE * hexstrm=NULL;

void StrUtil::HexDump(FILE * stream, void const * data, size_t len)
{
	unsigned int i;
	unsigned int r,c;

	if (!stream) return;
	if (!data) return;

	for (r=0,i=0; r<(len/16+(len%16!=0)); r++,i+=16)
	{
		fprintf(stream,"%04X:   ",i); /* location of first byte in line */

		for (c=i; c<i+8; c++){
			if (c<len)
				fprintf(stream,"%02X ",((unsigned char const *)data)[c]);
			else
				fprintf(stream,"   "); /* pad if short line */
		}
		fprintf(stream,"  ");

		for (c=i+8; c<i+16; c++){
			if (c<len)
				fprintf(stream,"%02X ",((unsigned char const *)data)[c]);
			else
				fprintf(stream,"   "); /* pad if short line */
		}

		fprintf(stream,"   ");

		for (c=i; c<i+16; c++){
			if (c<len)
				if (((unsigned char const *)data)[c]>=32 &&
					((unsigned char const *)data)[c]<127)
					fprintf(stream,"%c",((char const *)data)[c]);
				else
					fprintf(stream,"."); /* put this for non-printables */
			else
				fprintf(stream," "); /* pad if short line */
		}

		fprintf(stream,"\n");
	}
	fflush(stream);
}


void StrUtil::CopyToAnsi( char* dest, const TCHAR* src )
{
#ifdef UNICODE
	wcstombs( dest, src, MAX_STRING_SIZE );  //_tclen(src)+2 );
#else
	strcpy( dest, src );
#endif
}

void StrUtil::CopyToUnicode( wchar_t* dest, const TCHAR* src )
{
#ifdef UNICODE
	wcscpy( dest, src );	
#else
	mbstowcs( dest, src, _tclen(src)+1 );
#endif
}

void StrUtil::CopyUnicode2Ansi( char* dest, const wchar_t* src )
{
	wcstombs( dest, src, MAX_STRING_SIZE );  //_tclen(src)+2 );
}

void StrUtil::CopyAnsi2Unicode( wchar_t* dest, const char* src )
{
	mbstowcs( dest, src, strlen(src)+1 );
}

char* StrUtil::GetStaticAnsi64()
{
	static char temp[64];
	return temp;
}

char* StrUtil::GetStaticAnsi1024()
{
	static char temp[1024];
	return temp;
}

char* StrUtil::GetStaticAnsi4096()
{
	static char temp[4096];
	return temp;
}

const char*	StrUtil::AnsiFromTCHAR( const TCHAR* src )
{
#ifdef UNICODE
	static char temp[4096];
	CopyToAnsi( temp, src );
	return temp;
#else
	return src;
#endif
}

const  char* StrUtil::AnsiFromUnicode( const wchar_t* src )
{
	static char temp[4096];
	wcstombs( temp, src, MAX_STRING_SIZE );  //_tclen(src)+2 );
	return temp;
}

const wchar_t* StrUtil::UnicodeFromTCHAR( const TCHAR* src )
{
#ifdef UNICODE
	return const_cast<wchar_t*>(src);	
#else
	static wchar_t temp[2048];
	CopyToUnicode( temp, src );
	return temp;
#endif
}

const wchar_t* StrUtil::UnicodeFromAnsi( const char* src )
{
	static wchar_t temp[2048];
	mbstowcs( temp, src, strlen(src)+1 );
	return temp;
}


const TCHAR* StrUtil::TCHARFromAnsi( const char* src )
{
#ifdef UNICODE
	return UnicodeFromAnsi( src );
#else
	return src;
#endif

}

const TCHAR* StrUtil::TCHARFromUnicode( const wchar_t* src )
{
#ifdef UNICODE
	return src;
#else
	return AnsiFromUnicode( src );
#endif

}
 

void StrUtil::CopyAnsi2TCHAR( TCHAR* dest, const char* src )
{
#ifdef UNICODE
	mbstowcs( dest, src, strlen(src)+1 );
#else
	strncpy( dest, src, MAX_STRING_SIZE );
#endif	
}

void StrUtil::CopyTCHAR2Ansi( char* dest, const TCHAR* src )
{
#ifdef UNICODE
	wcstombs( dest, src, MAX_STRING_SIZE );  //_tclen(src)+2 );
#else
	strncpy( dest, src, MAX_STRING_SIZE );
#endif
}