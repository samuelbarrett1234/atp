#pragma once


/*

\file

\author Samuel Barrett

\brief This is the common header file to all parts of this library.

*/


#define BOOST_LOG_DYN_LINK
#include <cassert>
#include <boost/log/trivial.hpp>


#define ATP_CORE_LOG(level) BOOST_LOG_TRIVIAL(level)


#ifdef ATP_CORE_EXPORTS

#define ATP_CORE_API __declspec(dllexport)

#else

#define ATP_CORE_API __declspec(dllimport)

#endif


#ifdef _DEBUG

// set if we should be writing defensive code
#define ATP_CORE_DEFENSIVE

#endif


// only enable the checks below if we are in defensive mode
#ifdef ATP_CORE_DEFENSIVE

// assertion for checking invariants
#define ATP_CORE_ASSERT(expr) assert(expr)

// assertion for checking preconditions
#define ATP_CORE_PRECOND(expr) assert(expr)

#else

#define ATP_CORE_ASSERT(expr) ((void)0)
#define ATP_CORE_PRECOND(expr) ((void)0)

#endif


