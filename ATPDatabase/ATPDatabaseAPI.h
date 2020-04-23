#pragma once


/*

\file

\author Samuel Barrett

\brief This is the common header file to all parts of this library.

*/


#include <cassert>


#ifdef ATP_DATABASE_EXPORTS

#define ATP_DATABASE_API __declspec(dllexport)

#else

#define ATP_DATABASE_API __declspec(dllimport)

#endif


#ifdef _DEBUG

// set if we should be writing defensive code
#define ATP_DATABASE_DEFENSIVE

#endif


// only enable the checks below if we are in defensive mode
#ifdef ATP_DATABASE_DEFENSIVE

// assertion for checking invariants
#define ATP_DATABASE_ASSERT(expr) assert(expr)

// assertion for checking preconditions
#define ATP_DATABASE_PRECOND(expr) assert(expr)

#else

#define ATP_DATABASE_ASSERT(expr) ((void)0)
#define ATP_DATABASE_PRECOND(expr) ((void)0)

#endif


