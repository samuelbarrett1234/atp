#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions for loading solvers from the search
	settings file.
*/


#include "../ATPSearchAPI.h"
#include "../Interfaces/ISolver.h"
#include "../Interfaces/IHeuristic.h"
#include <boost/property_tree/ptree.hpp>


namespace atp
{
namespace search
{


class IteratorManager;  // forward declaration


/**
\brief Try to create a solver, based on the type given in `ptree`.

\returns Nullptr on failure, otherwise returns the solver object

\throws Anything that might be thrown by the ptree.
*/
SolverPtr try_create_solver(logic::KnowledgeKernelPtr p_ker,
	const boost::property_tree::ptree& ptree,
	HeuristicPtr p_heuristic,
	std::unique_ptr<IteratorManager> p_iter_mgr);


/**
\brief Try to create the iteration settings flags
*/
logic::IterSettings try_get_flags(
	const boost::property_tree::ptree& ptree);


/**
\brief Try to create an IterativeDeepeningSolver from the given ptree

\returns Nullptr on failure, otherwise returns the solver object

\throws Anything that might be thrown by the ptree.
*/
SolverPtr try_create_IDS(logic::KnowledgeKernelPtr p_ker,
	const boost::property_tree::ptree& ptree,
	logic::IterSettings settings,
	HeuristicPtr p_heuristic,
	std::unique_ptr<IteratorManager> p_iter_mgr);


}  // namespace search
}  // namespace atp


