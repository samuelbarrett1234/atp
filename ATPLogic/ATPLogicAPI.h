#pragma once


/*

ATPLogicAPI.h

This is the common header file to all parts of this library.

*/


#ifdef ATP_LOGIC_EXPORTS

#define ATP_LOGIC_API __declspec(dllexport)

#else

#define ATP_LOGIC_API __declspec(dllimport)

#endif


