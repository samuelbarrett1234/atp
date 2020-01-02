#pragma once


// Author: Samuel Barrett


#ifdef ATP_EXPORTS


#define ATP_API __declspec(dllexport)


#else


#define ATP_API __declspec(dllimport)


#endif


