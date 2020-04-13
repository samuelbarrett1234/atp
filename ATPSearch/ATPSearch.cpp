/**

\file

\author Samuel Barrett

*/


#include "ATPSearch.h"
#include <algorithm>
#include "Internal/IterativeDeepeningSolver.h"


namespace atp
{
namespace search
{


ATP_SEARCH_API SolverPtr create_solver(
	logic::KnowledgeKernelPtr p_ker,
	SolverType st,
	HeuristicCollection ms)
{
	switch (st)
	{
	case SolverType::ITERATIVE_DEEPENING_UNINFORMED:
		return std::make_shared<IterativeDeepeningSolver>(
			p_ker,
			/* max_depth */ 25,
			/* starting depth */ 2);
	default:
		return SolverPtr();
	}
}


}  // namespace search
}  // namespace atp


