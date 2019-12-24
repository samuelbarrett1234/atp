#pragma once


// Author: Samuel Barrett


#include <cassert>


#ifdef ATP_FORCE_CHECKS


#define ATP_CHECK_PRECOND(expr, msg) if(!(expr)) throw std::exception(msg)
#define ATP_CHECK_POSTCOND(expr, msg) if(!(expr)) throw std::exception(msg)
#define ATP_CHECK_INVARIANT(expr, msg) if(!(expr)) throw std::exception(msg)


#else


#define ATP_CHECK_PRECOND(expr, msg) assert((expr) && msg)
#define ATP_CHECK_POSTCOND(expr, msg) assert((expr) && msg)
#define ATP_CHECK_INVARIANT(expr, msg) assert((expr) && msg)


#endif


