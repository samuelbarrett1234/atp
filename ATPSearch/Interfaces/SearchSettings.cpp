/**

\file

\author Samuel Barrett

*/


#include "SearchSettings.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../Internal/IterativeDeepeningSolver.h"


namespace pt = boost::property_tree;


namespace atp
{
namespace search
{


/**
\brief Try to create an IterativeDeepeningSolver from the given ptree

\returns Nullptr on failure, otherwise returns the solver object

\throws Anything that might be thrown by the ptree.
*/
SolverPtr try_create_ids(logic::KnowledgeKernelPtr p_ker,
	const pt::ptree& ptree);


bool load_search_settings(logic::KnowledgeKernelPtr p_ker,
	std::istream& in, SearchSettings* p_out_settings)
{
	ATP_SEARCH_PRECOND(p_out_settings != nullptr);
	ATP_SEARCH_PRECOND(p_ker != nullptr);

	pt::ptree ptree;

	try
	{
		pt::read_json(in, ptree);

		p_out_settings->name = ptree.get<std::string>(
			"name", "Unnamed Search Settings");
		p_out_settings->desc = ptree.get<std::string>(
			"desc", "No description.");

		p_out_settings->max_steps = ptree.get<size_t>(
			"max-steps", 10);
		p_out_settings->step_size = ptree.get<size_t>(
			"step-size", 100);

		// defaults to empty string (representing the user not
		// wanting to create a solver) if type is not set
		const std::string solver_type = ptree.get<std::string>(
			"type", "");

		if (solver_type == "IterativeDeepeningSolver")
		{
			p_out_settings->p_solver = try_create_ids(
				p_ker, ptree);

			if (p_out_settings->p_solver == nullptr)
			{
				// failed to create the solver we wanted
				return false;
			}
		}
		// else if... [other solver types go here]
		else if (solver_type.empty())
		{
			// default solver is empty
			p_out_settings->p_solver.reset();
		}
		else  // if (!solver_type.empty())
		{
			return false;  // bad solver type
		}
	}
	catch(pt::ptree_error&)
	{
		// in case we had already loaded this
		p_out_settings->p_solver.reset();

		return false;
	}

	return true;
}


SolverPtr try_create_ids(logic::KnowledgeKernelPtr p_ker,
	const pt::ptree& ptree)
{
	return std::make_shared<IterativeDeepeningSolver>(
		p_ker, ptree.get<size_t>("max-depth", 10),
		ptree.get<size_t>("starting-depth", 3));
}


}  // namespace search
}  // namespace atp


