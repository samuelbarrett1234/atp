#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading heuristics from the search
	settings file.
*/


#include <functional>
#include <boost/property_tree/ptree.hpp>
#include "../ATPSearchAPI.h"
#include "../Interfaces/IHeuristic.h"


namespace atp
{
namespace search
{


typedef std::function<HeuristicPtr(
	const logic::KnowledgeKernelPtr&)> HeuristicCreator;


/**
\brief Try to create a heuristic object from the tree rooted at the
	given parameter.

\param creator If loading was successful, this functor will be set to
	be a creator for the heuristic.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_heuristic(
	const logic::ModelContextPtr& p_ctx,
	HeuristicCreator& creator,
	const boost::property_tree::ptree& ptree);


/**
\brief Try to create an edit distance heuristic object from the tree
	rooted at the given node.

\param creator If loading was successful, this functor will be set to
	be a creator for the heuristic.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_edit_distance_heuristic(
	const logic::ModelContextPtr& p_ctx,
	HeuristicCreator& creator,
	const boost::property_tree::ptree& ptree);


}  // namespace search
}  // namespace atp


