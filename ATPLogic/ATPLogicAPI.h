#pragma once


/**

\file

\author Samuel Barrett

\brief This is the common header file to all parts of this library.
    It defines several macros used throughout the library.

\details This file contains macro definitions for precondition
    checking, assertion checking, and DLL-export/import specification
    which is necessary to export and import functions and classes
    from this library.

*/


#include <cassert>


/**
\def ATP_LOGIC_EXPORTS
The export preprocessor definition is automatically defined when this
project is being built, but not when its files are being included in
other projects. This is because we only want to export when we're
building the library.
*/
#ifdef ATP_LOGIC_EXPORTS

/**
\def ATP_LOGIC_API
This needs to be added to all classes and functions that are going to
be DLL-exported.
*/
#define ATP_LOGIC_API __declspec(dllexport)

#else

#define ATP_LOGIC_API __declspec(dllimport)

#endif


#ifdef _DEBUG

/**
\def ATP_LOGIC_DEFENSIVE
Define this if we should be checking things defensively (this can be
expensive, however.) If defensive mode is enabled, you should also
enable the assertions and preconditions, if they are not already.
*/
#define ATP_LOGIC_DEFENSIVE

#endif


#ifdef ATP_LOGIC_DEFENSIVE

/**
\def ATP_LOGIC_ASSERT
Assert the truth of a given expression. This macro should be used in
cases where the expression should never be false, even if the library
user abuses preconditions etc. In other words, an assertion is, in a
sense, stronger than a precondition. 
*/
#define ATP_LOGIC_ASSERT(expr) assert(expr)

/**
\def ATP_LOGIC_PRECOND
Check that a given precondition holds. It might be desirable to leave
this macro enabled in release builds, to check that library users
aren't abusing preconditions etc.
*/
#define ATP_LOGIC_PRECOND(expr) assert(expr)

#else

#define ATP_LOGIC_ASSERT(expr) ((void)0)
#define ATP_LOGIC_PRECOND(expr) ((void)0)

#endif


