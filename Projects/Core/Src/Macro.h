#pragma once

namespace Core 
{

#ifdef SYSTEM_EXPORT
    // DLL library project uses this
    #define SYSTEM_ENTRY __declspec(dllexport)
#else
#ifdef SYSTEM_IMPORT
    // Application of DLL uses this
    #define SYSTEM_ENTRY __declspec(dllimport)
#else
    // static library project uses this
    #define SYSTEM_ENTRY
#endif
#endif


#define SAFE_DELETE(p)				{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)		{ if(p) { delete[] p;     p=NULL; } }


#ifndef IN
# define IN
#endif
#ifndef OUT
# define OUT
#endif

//Output Debug Msg in Compile Time
#define LINE1(x) #x
#define LINE(x) LINE1(x)
#define PrintCompileMsg(msg) __pragma (message( __FILE__"("LINE(__LINE__)"): [CompileMsg] " msg))


#define			VSNPRINTF		_vsntprintf

#define GET_VARARGS(msg,len,lastarg,fmt)	\
{	\
	va_list ArgPtr;	\
	va_start( ArgPtr, lastarg );	\
	VSNPRINTF( msg, len, fmt, ArgPtr );	\
	va_end( ArgPtr );	\
}

#define GET_VARARGS_ANSI(msg,len,lastarg,fmt)	\
{	\
	va_list ArgPtr;	\
	va_start( ArgPtr, lastarg );	\
	_vsnprintf( msg, len, fmt, ArgPtr );	\
	va_end( ArgPtr );	\
}
}