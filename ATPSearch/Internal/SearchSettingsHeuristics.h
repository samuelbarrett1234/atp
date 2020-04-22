#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading heuristics from the search
	settings file.
*/


#include "../ATPSearchAPI.h"
#include "../Interfaces/IHeuristic.h"
#include <boost/property_tree/ptree.hpp>


namespace atp
{
namespace search
{


/**
\brief Try to create a heuristic object from the tree rooted at the
	given parameter.

\returns Nullptr on failure, otherwise returns the heuristic object

\throws Anything that might be thrown by the ptree.
*/
HeuristicPtr try_create_heuristic(
	const boost::property_tree::ptree& ptree);


}  // namespace search
}  // namespace atp


