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


SolverPtr create_default_solver(
	logic::KnowledgeKernelPtr p_ker)
{
	return std::make_shared<IterativeDeepeningSolver>(
		p_ker, /* max depth */10, /* starting depth */ 2);
}


}  // namespace search
}  // namespace atp


