/**

\file

\author Samuel Barrett

*/


#include "RuleMatchingIterator.h"
#include "MatchResultsIterator.h"
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


std::shared_ptr<RuleMatchingIterator> RuleMatchingIterator::construct(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent,
	const Statement& forefront_stmt,
	const Statement::iterator& sub_expr,
	const std::vector<std::pair<size_t,
	 SyntaxNodeType>>& free_const_enum)
{
	return std::make_shared<RuleMatchingIterator>(ctx, ker,
		parent, forefront_stmt, sub_expr, free_const_enum);
}


RuleMatchingIterator::RuleMatchingIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent,
	const Statement& forefront_stmt,
	const Statement::iterator& sub_expr,
	const std::vector<std::pair<size_t,
	 SyntaxNodeType>>& free_const_enum) :
	m_ctx(ctx), m_ker(ker), m_parent(parent),
	m_forefront_stmt(forefront_stmt), m_sub_expr_iter(sub_expr),
	m_match_index(0), m_free_const_enum(free_const_enum)
{
	if (m_ker.num_matching_rules() > 0)
	{
		// construct m_cur_matching
		restore_invariant();
	}
}


bool RuleMatchingIterator::valid() const
{
	return m_match_index < m_ker.num_matching_rules();
}


ProofStatePtr RuleMatchingIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_cur_matching != nullptr);
	ATP_LOGIC_ASSERT(m_cur_matching->valid());

	return m_cur_matching->get();
}


void RuleMatchingIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_cur_matching != nullptr);
	ATP_LOGIC_ASSERT(m_cur_matching->valid());

	m_cur_matching->advance();

	if (!m_cur_matching->valid())
	{
		m_cur_matching.reset();
		++m_match_index;

		if (m_match_index < m_ker.num_matching_rules())
		{
			restore_invariant();
		}
	}
}


size_t RuleMatchingIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_cur_matching != nullptr);
	ATP_LOGIC_ASSERT(m_cur_matching->valid());
	return m_cur_matching->size();
}


void RuleMatchingIterator::restore_invariant()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_PRECOND(m_cur_matching == nullptr);

	std::map<size_t, Expression> match_subs;

	// keep trying to match
	while (valid() && !m_ker.try_match(m_match_index,
		*m_sub_expr_iter,
		&match_subs))
		++m_match_index;

	if (valid())
	{
		m_cur_matching = MatchResultsIterator::construct(
			m_ctx, m_ker, m_parent, m_forefront_stmt,
			m_ker.match_results_at(m_match_index,
				std::move(match_subs)),
			m_sub_expr_iter, m_free_const_enum);

		// this could only be invalid if there were no match results
		// from the kernel, but of course, there should be
		ATP_LOGIC_ASSERT(m_cur_matching->valid());
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


