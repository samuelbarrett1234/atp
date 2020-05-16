#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the SubExprMatchingIterator which implements
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
class RuleMatchingIterator;  // forward declaration


/**
\brief Produces successor statements by iterating over all subtrees
	of a statement to try matching.

\details An object for iterating over each possible subtree (of the
	forefront statement) and delegating each subtree to the
	RuleMatchingIterator which tries to match each sub expression
	to something.
*/
class ATP_LOGIC_API SubExprMatchingIterator :
	public IPfStateSuccIter
{
public:
	/**
	\note A lot of the data given in this constructor will just come
		directly from the ProofState that instantiated this iterator.

	\see atp::logic::equational::ProofState

	\param parent The statement we are ultimately trying to
		prove.

	\param forefront_stmt The statement at the "forefront" of the
		proof; the one we are producing successors of (and sub-
		expressions of).

	\param randomised Whether or not this iterator (and all iterators
		constructed by it in the process) should try to randomise
		their output as much as possible.

	\pre All free variable IDs in forefront_stmt are > `ker.
		get_rule_free_id_bound()`, so we don't need to worry about
		clashing of free variables when making substitutions later.
	*/
	static std::shared_ptr<SubExprMatchingIterator> construct(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const ProofState& parent,
		const Statement& forefront_stmt,
		bool randomised);

	/**
	\brief Try to refrain from using this version for creating a
		pointer; instead use `construct`. The arguments are the
		same.

	\see SubExprMatchingIterator::construct
	*/
	SubExprMatchingIterator(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const ProofState& parent,
		const Statement& forefront_stmt,
		bool randomised);

	bool valid() const override;
	ProofStatePtr get() const override;
	void advance() override;
	size_t size() const override;

private:
	/**
	\brief Used to create `m_rule_iters[m_cur_idx]`

	\pre m_rule_iters[m_cur_idx] == nullptr
	*/
	void construct_child();

	/**
	\brief Randomly generate a new m_cur_idx

	\post Could potentially cause !valid() but this is desirable as
		it fixes the invariant
	*/
	void reset_current();

private:
	// a flag; not used in this class but is passed on to children
	const bool m_randomised;

	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	const ProofState& m_parent;
	const Statement& m_forefront_stmt;

	// an enumeration of free variables found in `m_forefront_stmt`
	// and constant symbol IDs.
	// each pair represents (free or const ID, type).
	std::vector<std::pair<size_t, SyntaxNodeType>> m_free_const_enum;

	/*
	INVARIANTS: we have two different sets of invariants for random
	mode vs non-random mode

	COMMON INVARIANTS:
	we are valid iff m_cur_idx < m_rule_iters.size()

	NON-RANDOM MODE
	m_sub_expr_iters.size() == 1 holds the iterator over the subexprs
	m_rule_iters[m_cur_idx] is the only non-null element (and it is
	valid) whenever we are valid

	RANDOM MODE
	m_sub_expr_iters.size() == m_rule_iters.size()
	each m_rule_iters[i] is null or it is valid
	we are valid() iff there exists a null m_rule_iters[i] or a
	valid one (i.e we are invalid if all are non-null and invalid)
	m_rule_iters[m_cur_idx] is non-null and valid, or we are invalid
	*/

	std::vector<Statement::iterator> m_sub_expr_iters;
	std::vector<std::shared_ptr<RuleMatchingIterator>> m_rule_iters;
	size_t m_cur_idx;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


