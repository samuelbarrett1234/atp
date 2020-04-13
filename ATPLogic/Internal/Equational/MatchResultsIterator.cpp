/**

\file

\author Samuel Barrett

*/


#include "MatchResultsIterator.h"
#include "FreeVarAssignmentIterator.h"
#include "Expression.h"
#include "Statement.h"
#include "ModelContext.h"
#include "KnowledgeKernel.h"


namespace atp
{
namespace logic
{
namespace equational
{


PfStateSuccIterPtr MatchResultsIterator::construct(
	const ModelContext& ctx,
	const KnowledgeKernel& ker,
	const Statement& target_stmt,
	const Statement& forefront_stmt,
	const std::map<size_t, Expression>& match_subs,
	const std::vector<std::pair<Expression,
		std::vector<size_t>>>& match_results,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum)
{
	return std::make_shared<MatchResultsIterator>(
		ctx, ker, target_stmt, forefront_stmt, match_subs,
		match_results, free_const_enum);
}


MatchResultsIterator::MatchResultsIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const Statement& target_stmt, const Statement& forefront_stmt,
	const std::map<size_t, Expression>& match_subs,
	const std::vector<std::pair<Expression,
		std::vector<size_t>>>& match_results,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum) :
	m_ctx(ctx), m_ker(ker), m_target_stmt(target_stmt),
	m_forefront_stmt(forefront_stmt), m_match_subs(match_subs),
	m_match_results(match_results), m_match_result_index(0),
	m_free_const_enum(free_const_enum)
{
	// create a m_free_var_assignment
	restore_invariant();
}


bool MatchResultsIterator::valid() const
{
	return m_match_result_index < m_match_subs.size();
}


ProofStatePtr MatchResultsIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_free_var_assignment != nullptr);
	
	return m_free_var_assignment->get();
}


void MatchResultsIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_free_var_assignment != nullptr);
	ATP_LOGIC_ASSERT(m_free_var_assignment->valid());

	m_free_var_assignment->advance();

	if (!m_free_var_assignment->valid())
	{
		m_match_result_index++;
		m_free_var_assignment.reset();
		restore_invariant();
	}
}


size_t MatchResultsIterator::size() const
{
	if (!valid())
	{
		return 0;
	}
	else
	{
		ATP_LOGIC_ASSERT(m_free_var_assignment != nullptr);
		return m_free_var_assignment->size();
	}
}


void MatchResultsIterator::restore_invariant()
{
	ATP_LOGIC_PRECOND(m_free_var_assignment == nullptr);
	ATP_LOGIC_PRECOND(valid());

	// TODO: construct the effect of the substitution to pass
	// to the free var assignment iterator below

	m_free_var_assignment = FreeVarAssignmentIterator::construct(
		m_ctx, m_ker, m_target_stmt, subbed_stmt, m_free_const_enum,
		m_match_results[m_match_result_index].second.begin(),
		m_match_results[m_match_result_index].second.end());
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


