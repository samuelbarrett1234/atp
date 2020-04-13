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


PfStateSuccIterPtr RuleMatchingIterator::construct(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const Statement& target_stmt,
	const Statement& forefront_stmt)
{
	return std::make_shared<RuleMatchingIterator>(ctx, ker,
		target_stmt, forefront_stmt);
}


RuleMatchingIterator::RuleMatchingIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const Statement& target_stmt,
	const Statement& forefront_stmt) :
	m_ctx(ctx), m_ker(ker), m_target_stmt(target_stmt),
	m_forefront_stmt(forefront_stmt),
	m_match_index(0)
{
	// build the enumeration of free variable IDs in the forefront
	// and constant symbol IDs
	
	const auto free_ids = forefront_stmt.free_var_ids();
	const auto const_ids = ctx.all_constant_symbol_ids();

	m_free_const_enum.reserve(
		free_ids.size() + const_ids.size());

	for (auto id : free_ids)
		m_free_const_enum.emplace_back(id,
			SyntaxNodeType::FREE);

	for (auto id : const_ids)
		m_free_const_enum.emplace_back(id,
			SyntaxNodeType::CONSTANT);

	// now construct m_cur_matching
	restore_invariant();
}


bool RuleMatchingIterator::valid() const
{
	return m_match_index < m_ker.num_matching_rules();
}


ProofStatePtr RuleMatchingIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());

	return ProofStatePtr();
}


void RuleMatchingIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());

}


size_t RuleMatchingIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());

	return size_t();
}

void RuleMatchingIterator::restore_invariant()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_PRECOND(m_cur_matching == nullptr);

	m_cur_matching = MatchResultsIterator::construct(
		m_ker, m_ctx, m_target_stmt, m_forefront_stmt,

	)
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


