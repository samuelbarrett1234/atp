#pragma once


/*

\file

\author Samuel Barrett

\brief This is the common header file to all parts of this library.

*/


#include <cassert>


#ifdef ATP_STATS_EXPORTS

#define ATP_STATS_API __declspec(dllexport)

#else

#define ATP_STATS_API __declspec(dllimport)

#endif


#ifdef _DEBUG

// set if we should be writing defensive code
#define ATP_STATS_DEFENSIVE

#endif


// only enable the checks below if we are in defensive mode
#ifdef ATP_STATS_DEFENSIVE

// assertion for checking invariants
#define ATP_STATS_ASSERT(expr) assert(expr)

// assertion for checking preconditions
#define ATP_STATS_PRECOND(expr) assert(expr)

#else

#define ATP_STATS_ASSERT(expr) ((void)0)
#define ATP_STATS_PRECOND(expr) ((void)0)

#endif


/**
\namespace atp::stats

\brief This namespace contains the code behind the statistical
	models used in heuristics and other statistical applications
	elsewhere in the library.
*/


namespace atp
{
namespace stats
{





}  // namespace stats
}  // namespace atp


