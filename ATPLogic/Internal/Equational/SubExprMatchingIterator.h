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


class ModelContext;  // forward declaration
class KnowledgeKernel;  // forward declaration


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

	\param target_stmt The statement we are ultimately trying to
		prove.

	\param forefront_stmt The statement at the "forefront" of the
		proof; the one we are producing successors of (and sub-
		expressions of).

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

	\see SubExprMatchingIterator::construct
	*/
	SubExprMatchingIterator(const ModelContext& ctx,
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
		`m_rule_iter` by constructing it and giving
		it a value.

	\pre m_rule_iter == nullptr && valid()
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

	// invariants:
	// m_rule_iter is null or it is valid
	// we are valid() iff m_rule_iter is non-null

	Statement::iterator m_sub_expr_iter;
	PfStateSuccIterPtr m_rule_iter;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


