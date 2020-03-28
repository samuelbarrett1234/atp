#pragma once


/*

ATPDatabaseAPI.h

This is the common header file to all parts of this library.

*/


#ifdef ATP_DATABASE_EXPORTS

#define ATP_DATABASE_API __declspec(dllexport)

#else

#define ATP_DATABASE_API __declspec(dllimport)

#endif


