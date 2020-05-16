#pragma once


/*

\file

\author Samuel Barrett

\brief Contains includes and macros for logging

*/


#define BOOST_LOG_DYN_LINK
#include <boost/log/trivial.hpp>


#define ATP_SEARCH_LOG(level) BOOST_LOG_TRIVIAL(level)


