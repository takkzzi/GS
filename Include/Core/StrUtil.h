#pragma once
#include <cstdio>

namespace Core
{
	#define MAX_STRING_SIZE	(2048)

	class StrUtil
	{
	public : 
		
		//Hex Manipulating
		//@ 반드시 data 사이즈가 len보다 2배 이상이어야 한다.
		static void		Hex2Ascii(const unsigned char* hex, char* data, size_t len);
		static void		Ascii2Hex(const char* data, unsigned char* hex, size_t len);
		static void		HexDump(FILE* stream, void const * data, size_t len);
		//String Converting

		static void				CopyAnsi2TCHAR( TCHAR* dst, const char* src );
		static void				CopyTCHAR2Ansi( char* dst, const TCHAR* src );
		static void				CopyToAnsi( char* dest, const TCHAR* src );

		static void				CopyToUnicode( wchar_t* dest, const TCHAR* src );
		static void				CopyUnicode2Ansi( char* dest, const wchar_t* src);
		static void				CopyAnsi2Unicode( wchar_t* dest, const char* src );
		static char*			GetStaticAnsi64();
		static char*			GetStaticAnsi1024();
		static char*			GetStaticAnsi4096();
		static const char*		AnsiFromTCHAR( const TCHAR* src );
		static const char*		AnsiFromUnicode( const wchar_t* src );
		static const wchar_t* 	UnicodeFromTCHAR( const TCHAR* src );
		static const wchar_t* 	UnicodeFromAnsi( const char* src );
		static const TCHAR*		TCHARFromAnsi( const char* src );
		static const TCHAR*		TCHARFromUnicode( const wchar_t* src );
	};
}

