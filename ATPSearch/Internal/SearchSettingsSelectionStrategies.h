#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading selection strategies from the
	search settings files.

*/


#include <functional>
#include <boost/property_tree/ptree.hpp>
#include "../ATPSearchAPI.h"
#include "../Interfaces/ISelectionStrategy.h"


namespace atp
{
namespace search
{


// arguments are (model context ptr, model context ID)
typedef std::function<SelectionStrategyPtr(
	logic::ModelContextPtr, size_t /* ctx_id */
	)> SelectionStrategyCreator;


/**
\brief Try to create a selection strategy, based on the type given
	in `ptree`.

\param creator This function will be assigned a value iff success.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_selection_strategy(
	const boost::property_tree::ptree& ptree,
	SelectionStrategyCreator& creator);


/**
\brief Try to create a FixedSelectionStrategy, based on the type
	given in `ptree`.

\param creator This function will be assigned a value iff success.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_fixed_selection_strategy(
	const boost::property_tree::ptree& ptree,
	SelectionStrategyCreator& creator);


/**
\brief Try to create an EditDistanceSelectionStrategy, based on the
	type given in `ptree`.

\param creator This function will be assigned a value iff success.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_edit_dist_selection_strategy(
	const boost::property_tree::ptree& ptree,
	SelectionStrategyCreator& creator);


}  // namespace search
}  // namespace atp


