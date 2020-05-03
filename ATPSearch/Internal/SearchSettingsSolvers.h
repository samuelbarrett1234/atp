#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading solvers from the search
	settings file.
*/


#include <functional>
#include <boost/property_tree/ptree.hpp>
#include "../ATPSearchAPI.h"
#include "../Interfaces/ISolver.h"
#include "../Interfaces/IHeuristic.h"


namespace atp
{
namespace search
{


class IteratorManager;  // forward declaration
typedef std::function<SolverPtr(
	logic::KnowledgeKernelPtr,
	std::unique_ptr<IteratorManager>)> SolverCreator;


/**
\brief Try to create a solver, based on the type given in `ptree`.

\param creator This function will be assigned a value iff success.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_solver(
	const boost::property_tree::ptree& ptree,
	SolverCreator& creator);


/**
\brief Try to create the iteration settings flags

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API logic::IterSettings try_get_flags(
	const boost::property_tree::ptree& ptree);


/**
\brief Try to create an IterativeDeepeningSolver from the given ptree

\param creator This function will be assigned a value iff success.

\returns True iff success

\throws Anything that might be thrown by the ptree.
*/
ATP_SEARCH_API bool try_create_IDS(
	const boost::property_tree::ptree& ptree,
	SolverCreator& creator,
	logic::IterSettings iter_settings);


}  // namespace search
}  // namespace atp


