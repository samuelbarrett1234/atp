#pragma once


/*

ATPLogicAPI.h

This is the common header file to all parts of this library.

*/


#include <cassert>


#ifdef ATP_LOGIC_EXPORTS

#define ATP_LOGIC_API __declspec(dllexport)

#else

#define ATP_LOGIC_API __declspec(dllimport)

#endif


#ifdef _DEBUG

// set if we should be writing defensive code
#define ATP_LOGIC_DEFENSIVE

#endif


// only enable the checks below if we are in defensive mode
#ifdef ATP_LOGIC_DEFENSIVE

// assertion for checking invariants
#define ATP_LOGIC_ASSERT(expr) assert(expr)

// assertion for checking preconditions
#define ATP_LOGIC_PRECOND(expr) assert(expr)

#else

#define ATP_LOGIC_ASSERT(expr) ((void)0)
#define ATP_LOGIC_PRECOND(expr) ((void)0)

#endif


