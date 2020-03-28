#pragma once


/*

ATPSearchAPI.h

This is the common header file to all parts of this library.

*/


#ifdef ATP_SEARCH_EXPORTS

#define ATP_SEARCH_API __declspec(dllexport)

#else

#define ATP_SEARCH_API __declspec(dllimport)

#endif


