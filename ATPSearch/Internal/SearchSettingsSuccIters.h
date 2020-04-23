#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading stopping strategies from the
	search settings file.
*/


#include "../ATPSearchAPI.h"
#include "../Interfaces/IStoppingStrategy.h"
#include <boost/property_tree/ptree.hpp>


namespace atp
{
namespace search
{


class IteratorManager;  // forward declaration


/**
\brief Try to create a stopping strategy from the subtree rooted at
	the given node. Load them into the iterator manager.

\returns True iff success.

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_load_succ_iter_settings(
	IteratorManager& iter_mgr,
	const boost::property_tree::ptree& ptree);


/**
\brief Try to load info for a FixedStoppingStrategy

\returns True iff success.

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_load_fixed_stopping_strategy(
	IteratorManager& iter_mgr,
	const boost::property_tree::ptree& ptree);


/**
\brief Try to load info for a BasicStoppingStrategy

\returns True iff success.

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_load_basic_stopping_strategy(
	IteratorManager& iter_mgr,
	const boost::property_tree::ptree& ptree);


}  // namespace search
}  // namespace atp


