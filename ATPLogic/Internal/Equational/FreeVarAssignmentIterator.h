#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the FreeVarAssignmentIterator which implements
	`IPfStateSuccIter`.

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IProofState.h"
#include "Statement.h"
#include "SyntaxNodes.h"
#include "../FreeVarIdSet.h"
#include "MaybeRandomIndex.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ProofState;  // forward declaration
class Expression;  // forward declaration
class ModelContext;  // forward declaration
class KnowledgeKernel;  // forward declaration


/**
\brief Produces statement successors by iterating over possible
	assignments of free variables not covered by an existing mapping.

\details An object for iterating over all possible assignments of the
	remaining free variables that weren't covered by the partial
	mapping resulting from a match. This iterator works by picking a
	remaining free variable ID arbitrarily, and enumerating values
	for it, but also recursing (creating child
	FreeVarAssignmentIterator objects) to enumerate the remaining
	free variables, if there are any. If there are not any, then this
	implementation is trivial, and only has one item to iterate over.
*/
class ATP_LOGIC_API FreeVarAssignmentIterator :
	public IPfStateSuccIter
{
public:
	/**
	\note A lot of the data given in this constructor will just come
		directly from the MatchResultsIterator that instantiated this
		iterator.

	\see atp::logic::equational::MatchResultsIterator

	\param parent The statement we are ultimately trying to
		prove.

	\param subbed_stmt The statement which we will be substituting
		into, and (via the MatchResultsIterator) will have been
		produced as a result of a substitution.

	\param result The expression which we will be substituting into,
		which was previously the result of a match.

	\param free_const_enum An enumeration of all free variable IDs
		found in `forefront_stmt`, and all constant symbols.

	\param remaining_free_begin The start of an array of free IDs
		which are yet to be assigned.

	\param remaining_free_begin The end of an array of free IDs
		which are yet to be assigned.
	*/
	static std::shared_ptr<FreeVarAssignmentIterator> construct(
		const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const ProofState& parent,
		Statement subbed_stmt,
		const std::vector<std::pair<size_t,
			SyntaxNodeType>>& free_const_enum,
		FreeVarIdSet::const_iterator
			remaining_free_begin,
		FreeVarIdSet::const_iterator
			remaining_free_end,
		bool randomised);

	/**
	\brief Try to refrain from using this version for creating a
		pointer; instead use `construct`. The arguments are the
		same.

	\see FreeVarAssignmentIterator::construct
	*/
	FreeVarAssignmentIterator(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const ProofState& parent,
		Statement subbed_stmt,
		const std::vector<std::pair<size_t,
			SyntaxNodeType>>& free_const_enum,
		FreeVarIdSet::const_iterator
			remaining_free_begin,
		FreeVarIdSet::const_iterator
			remaining_free_end,
		bool randomised);

	bool valid() const override;
	ProofStatePtr get() const override;
	void advance() override;
	size_t size() const override;

private:
	/**
	\brief Used to restore the invariant around the variable
		`m_child` by constructing it and giving it a value.

	\pre m_child == nullptr && valid() && !m_is_leaf
	*/
	void restore_invariant();

private:
	const bool m_randomised;

	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	const ProofState& m_parent;
	Statement m_subbed_stmt;

	// set to (m_remaining_free_begin == m_remaining_free_end)
	const bool m_is_leaf;
	ProofStatePtr m_leaf_result;  // only set for leaves

	// only used when m_is_leaf is true, and represents whether or
	// not we have advanced (if we are a leaf,
	// then valid()==!m_leaf_is_done)
	bool m_leaf_is_done;

	// the range of free IDs left to cover; we will be enumerating
	// values to the **FIRST** element in this range, if it is
	// nonempty.
	// we are a leaf iff these iterators are equal
	const FreeVarIdSet::const_iterator
		m_remaining_free_begin, m_remaining_free_end;

	// the enumeration of values we will be substituting for our
	// free variable (not used for leaves)
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& m_free_const_enum;

	// the index of the value we currently have to substitute for our
	// free variable
	MaybeRandomIndex m_cur_free_idx;

	// if there are other free variables in the remaining free
	// variable range that we have not covered, we must also
	// enumerate those values too.
	// if m_child is not null then it is valid
	std::shared_ptr<FreeVarAssignmentIterator> m_child;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


