#pragma once


/**
\file

\author Samuel Barrett

\brief Contains assertion macros for `atp` project.

*/


#define BOOST_LOG_DYN_LINK
#include <cassert>
#include <boost/log/trivial.hpp>


#define ATP_LOG(level) BOOST_LOG_TRIVIAL(level)


#ifdef _DEBUG

/**
\def ATP_ASSERT
Assert the truth of a given expression. This macro should be used in
cases where the expression should never be false, even if the app
user abuses preconditions etc. In other words, an assertion is, in a
sense, stronger than a precondition.
*/
#define ATP_ASSERT(expr) assert(expr)

/**
\def ATP_PRECOND
Check that a given precondition holds. It might be desirable to leave
this macro enabled in release builds, to check that application users
aren't abusing preconditions etc.
*/
#define ATP_PRECOND(expr) assert(expr)

#else

#define ATP_ASSERT(expr) ((void)0)
#define ATP_PRECOND(expr) ((void)0)

#endif


