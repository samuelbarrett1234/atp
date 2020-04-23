/*
\file

\author Samuel Barrett
*/


#include "SearchSettingsSolvers.h"
#include "IterativeDeepeningSolver.h"
#include "IteratorManager.h"
#include <ATPLogic.h>


namespace atp
{
namespace search
{


SolverPtr try_create_solver(logic::KnowledgeKernelPtr p_ker,
	const boost::property_tree::ptree& ptree, HeuristicPtr p_heuristic,
	std::unique_ptr<IteratorManager> p_iter_mgr)
{
	const std::string solver_type = ptree.get<std::string>(
		"type");

	auto iter_settings = try_get_flags(ptree);

	if (solver_type == "IterativeDeepeningSolver")
	{
		// may or may not fail
		return try_create_IDS(
			p_ker, ptree, iter_settings,
			p_heuristic, std::move(p_iter_mgr));
	}
	else
	{
		return nullptr;  // bad solver type
	}
}


logic::IterSettings try_get_flags(
	const boost::property_tree::ptree& ptree)
{
	logic::IterSettings flags = logic::iter_settings::DEFAULT;

	// each of these flags is optional to provide, and if it is
	// provided, it should either be `true` or `false`.

	if (auto b = ptree.get_optional<bool>("no-repeats"))
	{
		if (b.get())
			flags |= logic::iter_settings::NO_REPEATS;
		else
			flags &= ~logic::iter_settings::NO_REPEATS;
	}

	if (auto b = ptree.get_optional<bool>("randomised"))
	{
		if (b.get())
			flags |= logic::iter_settings::RANDOMISED;
		else
			flags &= ~logic::iter_settings::RANDOMISED;
	}

	return flags;
}


SolverPtr try_create_IDS(logic::KnowledgeKernelPtr p_ker,
	const boost::property_tree::ptree& ptree,
	logic::IterSettings settings,
	HeuristicPtr p_heuristic,
	std::unique_ptr<IteratorManager> p_iter_mgr)
{
	const size_t max_depth = ptree.get<size_t>("max-depth", 10);
	const size_t starting_depth = ptree.get<size_t>("starting-depth", 3);

	if (starting_depth <= 1 || starting_depth >= max_depth)
		return SolverPtr();  // invalid params

	return std::make_shared<IterativeDeepeningSolver>(
		p_ker, max_depth, starting_depth, settings,
		std::move(p_iter_mgr));
}


}  // namespace search
}  // namespace atp


