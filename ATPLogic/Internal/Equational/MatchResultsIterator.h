#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the MatchResultsIterator which implements
	`IPfStateSuccIter`.

*/


#include <map>
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
\brief Produces statement successors by iterating over the results of
	a successful matching.

\details An object for iterating over all the possible results from a
	successful matching. For each matching result, the iterator
	delegates the (potentially partial) free variable mapping to
	the FreeVarAssignmentIterator.
*/
class ATP_LOGIC_API MatchResultsIterator :
	public IPfStateSuccIter
{
public:
	/**
	\note A lot of the data given in this constructor will just come
		directly from the RuleMatchingIterator that instantiated this
		iterator.

	\see atp::logic::equational::RuleMatchingIterator

	\param target_stmt The statement we are ultimately trying to
		prove.

	\param forefront_stmt The statement at the "forefront" of the
		proof; the one we are producing successors of.

	\param match_subs The substitutions which caused the successful
		match.

	\param match_results The array of (Expression, Expression Free
		IDs) which resulted from the matching, and with which we
		are tasked of iterating over.

	\param free_const_enum An enumeration of all free variable IDs
		found in `forefront_stmt`, and all constant symbols.
	*/
	static PfStateSuccIterPtr construct(
		const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const Statement& target_stmt,
		const Statement& forefront_stmt,
		const std::map<size_t, Expression>& match_subs,
		const std::vector<std::pair<Expression,
			std::vector<size_t>>>& match_results,
		const std::vector<std::pair<size_t,
			SyntaxNodeType>>& free_const_enum);

	/**
	\brief Try to refrain from using this version for creating a
		pointer; instead use `construct`. The arguments are the
		same.

	\see MatchResultsIterator::construct
	*/
	MatchResultsIterator(const ModelContext& ctx,
		const KnowledgeKernel& ker,
		const Statement& target_stmt,
		const Statement& forefront_stmt,
		const std::map<size_t, Expression>& match_subs,
		const std::vector<std::pair<Expression,
			std::vector<size_t>>>& match_results,
		const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum);

	bool valid() const override;
	ProofStatePtr get() const override;
	void advance() override;
	size_t size() const override;

private:
	/**
	\brief Used to restore the invariant around the variable
		`m_free_var_assignment` by constructing it and giving
		it a value.

	\pre m_free_var_assignment == nullptr && valid()
	*/
	void restore_invariant();

private:
	const ModelContext& m_ctx;
	const KnowledgeKernel& m_ker;
	const Statement& m_target_stmt;
	const Statement& m_forefront_stmt;
	
	const std::map<size_t, Expression>& m_match_subs;

	const std::vector<std::pair<Expression,
		std::vector<size_t>>>& m_match_results;

	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& m_free_const_enum;

	// invariant: this index is the result we are currently pointing
	// to, and if it is out-of-bounds then this iterator is invalid.
	// m_free_var_assignment is never invalid, and is only null when
	// we are invalid.

	size_t m_match_result_index;
	PfStateSuccIterPtr m_free_var_assignment;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


