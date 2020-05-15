#pragma once


/**
\file

\author Samuel Barrett

\brief Contains functions relating to the selection of unproven
	theorems from the database which may be worth trying to prove.
*/


#include <map>
#include <string>
#include <memory>
#include <vector>
#include <boost/variant2/variant.hpp>
#include <ATPLogic.h>
#include "ATPStatsAPI.h"


namespace atp
{
namespace stats
{


/**
\typedef ThmParam

\typedef ThmParams

\brief A set of theorem parameters to use for calculating how much it
	is worth attempting a proof of that theorem.

\details The allowed parameters are as follows:
	- 'stmt' textual representation of the target theorem
	- 'thm_date' the Julian day representation of the date it was added
	- 'time_cost' the total amount of time spent proving it already
	- 'max_mem' the maximum amount of memory spent on any previous attempt
	- 'num_expansions' the total number of node expansions on this theorem
	- 'deadline' the Julian day representation of the target deadline
	- 'priority' a real-valued priority value for getting a proof of this statement
*/


typedef boost::variant2::variant<float, size_t, std::string> ThmParam;
typedef std::map<std::string, ThmParam> ThmParams;


/**
\brief This interface allows you to select theorems which might be
	most worth proving out of a given set of unproven theorems.

\details This class works in two phases: first, you input all of the
	available theorems into it via `add_thm`, then you call
	`calc_best` once you've added them all to allow it to perform its
	computations, then you use `read_next` to read out the selected
	indices one by one.
*/
class ATP_STATS_API IPfTargetSelector
{
public:
	virtual ~IPfTargetSelector() = default;

	/**
	\brief Clear any computation results and added theorems. Reset
		this object to a state where it is ready to accept theorems.
	*/
	virtual void reset() = 0;

	/**
	\brief Add a new theorem by providing a map of theorem parameters

	\pre Either `calc_best` has never been called, or `reset` has
		been called more recently than `calc_best`.
	*/
	virtual void add_thm(ThmParams params) = 0;

	/**
	\brief Indicates that the user wants to stop adding theorems, and
		prepare to extract the indices of the best statements.
	*/
	virtual void calc_best() = 0;

	/**
	\brief Extract the next best index (where the indices are
		corresponding to the order that the theorems were added using
		`add_thm`.

	\pre `calc_best` has been called more recently than `reset`.

	\pre p_out_idx != nullptr

	\post Exactly one of the following happens: we return false, or
		we write a value into `p_out_idx`.

	\returns True if a value was read, false if reading has finished.
	*/
	virtual bool read_next(size_t* p_out_idx) = 0;

	/**
	\brief Get a list of supported parameter names for ThmParams
	*/
	virtual std::vector<std::string> supported_param_names() const = 0;
};
typedef std::shared_ptr<IPfTargetSelector> PfTargetSelectorPtr;


/**
\brief Create a proof target selector object
*/
ATP_STATS_API PfTargetSelectorPtr create_pf_target_selector(
	const logic::ModelContextPtr& ctx);


}  // namespace stats
}  // namespace atp


