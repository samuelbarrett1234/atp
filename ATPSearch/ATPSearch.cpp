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
			/* max_depth */ 25);
	default:
		return SolverPtr();
	}
}


bool simple_check_proof(logic::IKnowledgeKernel& ker,
	logic::StatementArrayPtr proof)
{
	if (proof->size() == 0)
		return true;  // empty proof trivially true

	auto result = ker.follows(
		proof->slice(0, proof->size() - 1),
		proof->slice(1, proof->size()));

	return std::all_of(result.begin(), result.end(),
		[](bool x) { return x; })
		&& ker.get_form(proof->slice(0, 1)).front()
		== logic::StmtForm::CANONICAL_TRUE;
}


}  // namespace search
}  // namespace atp


