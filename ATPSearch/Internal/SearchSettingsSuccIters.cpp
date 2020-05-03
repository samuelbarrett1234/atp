/*
\file

\author Samuel Barrett
*/


#include "SearchSettingsSuccIters.h"
#include "IteratorManager.h"


namespace atp
{
namespace search
{


bool try_load_succ_iter_settings(SuccIterCreator& creator,
	const boost::property_tree::ptree& ptree)
{
	const std::string type = ptree.get<std::string>("type");

	if (type == "FixedStoppingStrategy")
	{
		return try_load_fixed_stopping_strategy(creator, ptree);
	}
	else if (type == "BasicStoppingStrategy")
	{
		return try_load_basic_stopping_strategy(creator, ptree);
	}
	else
	{
		return false;  // invalid type
	}
}


bool try_load_fixed_stopping_strategy(SuccIterCreator& creator,
	const boost::property_tree::ptree& ptree)
{
	const size_t size = ptree.get<size_t>("size");

	if (size == 0)
		return false;

	creator = boost::bind(
		&IteratorManager::set_fixed_stopping_strategy,
		_1, size);

	return true;
}


bool try_load_basic_stopping_strategy(SuccIterCreator& creator,
	const boost::property_tree::ptree& ptree)
{
	const size_t initial_size = ptree.get<size_t>("initial-size", 3);
	const float lambda = ptree.get<float>("lambda");
	const float alpha = ptree.get<float>("alpha");

	if (initial_size <= 1 || lambda <= 0.0f || alpha <= 0.0f
		|| alpha >= 1.0f)
		return false;

	creator = boost::bind(
		&IteratorManager::set_basic_stopping_strategy,
		_1, initial_size, lambda, alpha);

	return true;
}


}  // namespace search
}  // namespace atp


