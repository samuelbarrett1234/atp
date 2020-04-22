/*
\file

\author Samuel Barrett
*/


#include "SearchSettingsStoppingStrategies.h"
#include "IteratorManager.h"


namespace atp
{
namespace search
{


bool try_load_stopping_strategies(IteratorManager& iter_mgr,
	const boost::property_tree::ptree& ptree)
{
	const std::string type = ptree.get<std::string>("type");

	if (type == "FixedStoppingStrategy")
	{
		return try_load_fixed_stopping_strategy(iter_mgr, ptree);
	}
	else if (type == "BasicStoppingStrategy")
	{
		return try_load_basic_stopping_strategy(iter_mgr, ptree);
	}
	else
	{
		return false;  // invalid type
	}
}


bool try_load_fixed_stopping_strategy(IteratorManager& iter_mgr,
	const boost::property_tree::ptree& ptree)
{
	const size_t size = ptree.get<size_t>("size");

	if (size <= 1)
		return false;

	iter_mgr.set_fixed_stopping_strategy(size);

	return true;
}


bool try_load_basic_stopping_strategy(IteratorManager& iter_mgr,
	const boost::property_tree::ptree& ptree)
{
	const size_t initial_size = ptree.get<size_t>("initial-size", 3);
	const float lambda = ptree.get<float>("lambda");
	const float alpha = ptree.get<float>("alpha");

	if (initial_size <= 1 || lambda <= 0.0f || alpha <= 0.0f
		|| alpha >= 1.0f)
		return false;

	iter_mgr.set_basic_stopping_strategy(initial_size, lambda,
		alpha);

	return true;
}


}  // namespace search
}  // namespace atp


