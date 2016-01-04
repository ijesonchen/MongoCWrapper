// for windows platform, MSVC, use afx.h & DEBUG_NEW to detect memory leak.
// mfcnew.h MUST be last .h file included ONLY in .cpp

#if defined( _WIN32 ) || defined (WIN32)  
	#if defined(_MSC_VER) && defined(_DEBUG) && defined( _AFXDLL ) && defined(_DLL)
		#define __AFX_DEBUG_NEW__
	#endif
	#if defined(_MSC_VER) && defined(_DEBUG) && !defined( _AFXDLL ) && !defined(_DLL)
		#define __AFX_DEBUG_NEW__
	#endif
#endif

#ifdef __AFX_DEBUG_NEW__
	#pragma message("Message: **** mfc DEBUG_NEW is defined to help detect memory leakage.")
	#define new DEBUG_NEW
#else
	#ifdef _MSC_VER
		#pragma message("Message: **** mfc DEBUG_NEW not defined. Memory leakage may not detected.")
	#endif // _MSC_VER
#endif // __AFX_DEBUG_NEW__

#ifdef _MSC_VER
	#ifdef __AFXSTR_H__
		#pragma message("Message: **** mfc CString is used, related function is compiled.")
	#else
		#pragma message("Message: **** mfc CString NOT used, related function NOT compiled.")
	#endif // __AFXSTR_H__
#endif // _MSC_VER

#ifdef _MSC_VER
	#pragma warning (pop)
	// test inline state only
	#pragma warning(4:4711)
#endif // _MSC_VER