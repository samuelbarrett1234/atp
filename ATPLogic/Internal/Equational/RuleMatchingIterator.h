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
#include "MaybeRandomIndex.h"


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
class ATP_LOGIC_API RuleMatchingIterator
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
			SyntaxNodeType>>& free_const_enum,
		bool randomised);

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
			SyntaxNodeType>>& free_const_enum,
		bool randomised);

	bool valid() const;
	ProofStatePtr get() const;
	void advance();
	size_t size() const;

private:
	/**
	\brief Create a child iterator for the current position

	\note The created child may not be valid, so you'll need to check
		this.

	\pre m_matchings[m_match_index] == nullptr &&
		m_match_index < m_matchings.size() &&
		!m_is_no_matching[m_match_index]

	\post m_matchings[m_match_index] != nullptr ||
		m_is_no_matching[m_match_index]
	*/
	void rebuild_current();

private:
	// whether we should iterate over the rules in a random order
	// (this is also passed to children).
	const bool m_randomised;

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

	// invariant:
	// m_matchings[i] can be null/not null, valid/invalid
	// we are valid iff m_match_index == m_matchings.size()
	// invariant: m_matchings.size() == m_is_no_matching.size()
	// if m_is_no_matching[i] then m_matchings[i] is null,
	// we are invalid iff for all i, m_matchings[i] is non-null
	// and invalid or m_is_no_matching[i] is true.
	// finally, if m_match_index < m_matchings.size(), then
	// m_matchings[m_match_index] is non-null and valid, and in
	// particular m_is_no_matching[m_match_index] is false.

	size_t m_match_index;
	std::vector<std::shared_ptr<MatchResultsIterator>> m_matchings;
	std::vector<bool> m_is_no_matching;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


