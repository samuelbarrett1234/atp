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


bool try_create_solver(
	const boost::property_tree::ptree& ptree,
	SolverCreator& creator)
{
	const std::string solver_type = ptree.get<std::string>(
		"type");

	auto iter_settings = try_get_flags(ptree);

	if (solver_type == "IterativeDeepeningSolver")
	{
		// may or may not fail
		return try_create_IDS(ptree, creator, iter_settings);
	}
	else
	{
		return false;  // bad solver type
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


bool try_create_IDS(
	const boost::property_tree::ptree& ptree,
	SolverCreator& creator,
	logic::IterSettings iter_settings)
{
	const size_t max_depth = ptree.get<size_t>(
		"max-depth", 10);
	const size_t starting_depth = ptree.get<size_t>(
		"starting-depth", 3);
	const size_t width_limit = ptree.get<size_t>(
		"width-limit", 50);
	const size_t width_limit_start_depth = ptree.get<size_t>(
		"width-limit-start-depth", 2);

	if (starting_depth <= 1 || starting_depth >= max_depth ||
		width_limit == 0)
		return false;  // invalid params

	creator = [max_depth, starting_depth, width_limit,
		width_limit_start_depth, iter_settings](
		logic::KnowledgeKernelPtr p_ker,
		std::unique_ptr<IteratorManager> p_iter_mgr)
	{
		return std::make_shared<IterativeDeepeningSolver>(
			p_ker, max_depth, starting_depth, width_limit,
			width_limit_start_depth, iter_settings,
			std::move(p_iter_mgr));
	};

	return true;
}


}  // namespace search
}  // namespace atp


