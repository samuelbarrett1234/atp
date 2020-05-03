#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading stopping strategies from the
	search settings file.
*/


#include <functional>
#include <boost/property_tree/ptree.hpp>
#include "../ATPSearchAPI.h"
#include "../Interfaces/IStoppingStrategy.h"


namespace atp
{
namespace search
{


class IteratorManager;  // forward declaration
typedef std::function<void(IteratorManager&)> SuccIterCreator;


/**
\brief Try to create a stopping strategy from the subtree rooted at
	the given node. Load them into the iterator manager.

\param creator This function will be assigned a value iff success.

\returns True iff success.

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_load_succ_iter_settings(
	SuccIterCreator& creator,
	const boost::property_tree::ptree& ptree);


/**
\brief Try to load info for a FixedStoppingStrategy

\param creator This function will be assigned a value iff success.

\returns True iff success.

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_load_fixed_stopping_strategy(
	SuccIterCreator& creator,
	const boost::property_tree::ptree& ptree);


/**
\brief Try to load info for a BasicStoppingStrategy

\param creator This function will be assigned a value iff success.

\returns True iff success.

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_load_basic_stopping_strategy(
	SuccIterCreator& creator,
	const boost::property_tree::ptree& ptree);


}  // namespace search
}  // namespace atp


