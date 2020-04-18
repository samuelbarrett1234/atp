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


std::shared_ptr<MatchResultsIterator> MatchResultsIterator::construct(
	const ModelContext& ctx,
	const KnowledgeKernel& ker,
	const ProofState& parent,
	const Statement& forefront_stmt,
	std::vector<std::pair<Expression,
		FreeVarIdSet>> match_results,
	const Statement::iterator& sub_expr,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum)
{
	return std::make_shared<MatchResultsIterator>(
		ctx, ker, parent, forefront_stmt,
		std::move(match_results), sub_expr, free_const_enum);
}


MatchResultsIterator::MatchResultsIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent, const Statement& forefront_stmt,
	std::vector<std::pair<Expression,
		FreeVarIdSet>> match_results,
	const Statement::iterator& sub_expr,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum) :
	m_ctx(ctx), m_ker(ker), m_parent(parent),
	m_forefront_stmt(forefront_stmt),
	m_match_results(std::move(match_results)), m_match_result_index(0),
	m_free_const_enum(free_const_enum), m_sub_expr_iter(sub_expr)
{
	if (!m_match_results.empty())
	{
		// create a m_free_var_assignment
		restore_invariant();
	}
}


bool MatchResultsIterator::valid() const
{
	return m_match_result_index < m_match_results.size();
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
		m_free_var_assignment.reset();
		++m_match_result_index;

		if (m_match_result_index < m_match_results.size())
		{
			restore_invariant();
		}
	}
}


size_t MatchResultsIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_free_var_assignment != nullptr);
	ATP_LOGIC_ASSERT(m_free_var_assignment->valid());

	return m_free_var_assignment->size();
}


void MatchResultsIterator::restore_invariant()
{
	ATP_LOGIC_PRECOND(m_free_var_assignment == nullptr);
	ATP_LOGIC_PRECOND(valid());

	// replace a particular location in the forefront statement
	// with the match result at the given index, with the matching
	// substitution already applied
	Statement subbed_stmt = m_forefront_stmt.replace(m_sub_expr_iter,
		m_match_results[m_match_result_index].first);

	m_free_var_assignment = FreeVarAssignmentIterator::construct(
		m_ctx, m_ker, m_parent, std::move(subbed_stmt),
		m_free_const_enum,
		m_match_results[m_match_result_index].second.begin(),
		m_match_results[m_match_result_index].second.end());
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


