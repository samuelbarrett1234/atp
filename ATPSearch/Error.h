#pragma once


// Author: Samuel Barrett


#include <cassert>


#define ATP_CHECK_PRECOND(expr, msg) assert((expr) && msg)
#define ATP_CHECK_POSTCOND(expr, msg) assert((expr) && msg)
#define ATP_CHECK_INVARIANT(expr, msg) assert((expr) && msg)


