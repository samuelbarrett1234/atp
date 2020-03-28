#pragma once


/*

ATPSearchAPI.h

This is the common header file to all parts of this library.

*/


#include <cassert>


#ifdef ATP_SEARCH_EXPORTS

#define ATP_SEARCH_API __declspec(dllexport)

#else

#define ATP_SEARCH_API __declspec(dllimport)

#endif


#ifdef _DEBUG

#define ATP_SEARCH_ASSERT(expr) assert(expr)
#define ATP_SEARCH_PRECOND(expr) assert(expr)

#else

#define ATP_SEARCH_ASSERT(expr) ((void)0)
#define ATP_SEARCH_PRECOND(expr) ((void)0)

#endif
