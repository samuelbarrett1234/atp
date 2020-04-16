#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the RuleMatchingIterator which implements
	`IPfStateSuccIter`.

*/


#include <vector>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IProofState.h"
#include "Expression.h"
#include "Statement.h"
#include "SyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ProofState;  // forward declaration
class ModelContext;  // forward declaration
class KnowledgeKernel;  // forward declaration
class MatchResultsIterator;  // forward declaration


/**
\brief Produces successor statements by iterating over possible
	matchings.

\details An object for iterating over each possible matching (listed
	by the knowledge kernel) and outputting the results. For each
	matching found, it delegates to the MatchResultsIterator.
*/
class ATP_LOGIC_API RuleMatchingIterator :
	public IPfStateSuccIter
{
public:
	/**
	\note A lot of the data given in this constructor will just come
		directly from the SubExprMatchingIterator that instantiated
		this iterator.

	\see atp::logic::equational::SubExprMatchingIterator

	\param parent The statement we are ultimately trying to
		prove.

	\param forefront_stmt The statement at the "forefront" of the
		proof; the one we are producing successors of.

	\param sub_expr The iterator of the sub-expression we should
		try matching.

	\param free_const_enum An enumeration of all free variable IDs
		found in `forefront_stmt`, and all constant symbols.
	*/
	static std::shared_ptr<RuleMatchingIterator> construct(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const ProofState& parent,
		const Statement& forefront_stmt,
		const Statement::iterator& sub_expr,
		const std::vector<std::pair<size_t,
			SyntaxNodeType>>& free_const_enum);

	/**
	\brief Try to refrain from using this version for creating a
		pointer; instead use `construct`. The arguments are the
		same.

	\see RuleMatchingIterator::construct
	*/
	RuleMatchingIterator(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const ProofState& parent,
		const Statement& forefront_stmt,
		const Statement::iterator& sub_expr,
		const std::vector<std::pair<size_t,
			SyntaxNodeType>>& free_const_enum);

	bool valid() const override;
	ProofStatePtr get() const override;
	void advance() override;
	size_t size() const override;

private:
	/**
	\brief Used to restore the invariant around the variable
		`m_cur_matching` by constructing it and giving
		it a value.

	\pre m_cur_matching == nullptr && valid()
	*/
	void restore_invariant();

private:
	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	const ProofState& m_parent;
	const Statement& m_forefront_stmt;

	const Statement::iterator& m_sub_expr_iter;

	// an enumeration of free variables found in `m_forefront_stmt`
	// and constant symbol IDs.
	// each pair represents (free or const ID, type).
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& m_free_const_enum;

	// invariants:
	// m_cur_matching is null or it is valid
	// we are valid() iff m_cur_matching is non-null

	size_t m_match_index;
	std::shared_ptr<MatchResultsIterator> m_cur_matching;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


