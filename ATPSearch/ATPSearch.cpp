#include "ATPSearch.h"
#include <algorithm>


namespace atp
{
namespace search
{


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


