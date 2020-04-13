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
#include "SyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


class Expression;  // forward declaration
class Statement;  // forward declaration
class ModelContext;  // forward declaration
class KnowledgeKernel;  // forward declaration


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
		directly from the ProofState that instantiated this iterator.

	\see atp::logic::equational::ProofState

	\param target_stmt The statement we are ultimately trying to
		prove.

	\param forefront_stmt The statement at the "forefront" of the
		proof; the one we are producing successors of.

	\pre All free variable IDs in forefront_stmt are > `ker.
		get_rule_free_id_bound()`, so we don't need to worry about
		clashing of free variables when making substitutions later.
	*/
	static PfStateSuccIterPtr construct(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const Statement& target_stmt,
		const Statement& forefront_stmt);

	/**
	\brief Try to refrain from using this version for creating a
		pointer; instead use `construct`. The arguments are the
		same.

	\see RuleMatchingIterator::construct
	*/
	RuleMatchingIterator(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const Statement& target_stmt,
		const Statement& forefront_stmt);

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
	const Statement& m_target_stmt;
	const Statement& m_forefront_stmt;

	// an enumeration of free variables found in `m_forefront_stmt`
	// and constant symbol IDs.
	// each pair represents (free or const ID, type).
	std::vector<std::pair<size_t, SyntaxNodeType>> m_free_const_enum;

	// invariant: if m_cur_matching is null then the next thing this
	// iterator has to do is evaluate the matching at m_match_index,
	// if it is valid. If m_cur_matching is not null, then it is
	// valid, and we are currently iterating over m_match_index's
	// matching.
	// if m_match_index is out of bounds, then this iterator is no
	// longer valid, and m_cur_matching is nullptr.

	size_t m_match_index;
	PfStateSuccIterPtr m_cur_matching;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


