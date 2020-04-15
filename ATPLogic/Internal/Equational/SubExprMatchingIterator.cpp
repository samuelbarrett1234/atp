/**

\file

\author Samuel Barrett

*/


#include "SubExprMatchingIterator.h"
#include "RuleMatchingIterator.h"
#include "Expression.h"
#include "Statement.h"
#include "ModelContext.h"
#include "KnowledgeKernel.h"
#include <boost/bind.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


PfStateSuccIterPtr SubExprMatchingIterator::construct(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const Statement& target_stmt, const Statement& forefront_stmt)
{
	return std::make_shared<SubExprMatchingIterator>(ctx, ker,
		target_stmt, forefront_stmt);
}


SubExprMatchingIterator::SubExprMatchingIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const Statement& target_stmt, const Statement& forefront_stmt) :
	m_ctx(ctx), m_ker(ker), m_target_stmt(target_stmt),
	m_forefront_stmt(forefront_stmt),
	m_sub_expr_iter(forefront_stmt.begin())
{
	// build the enumeration of free variable IDs in the forefront
	// and constant symbol IDs

	const auto free_ids = forefront_stmt.free_var_ids();
	const auto const_ids = ctx.all_constant_symbol_ids();

	// check the precondition about the free variable IDs if we're
	// being defensive
	ATP_LOGIC_PRECOND(std::all_of(free_ids.begin(), free_ids.end(),
		boost::bind(std::less<size_t>(),
			ker.get_rule_free_id_bound(), _1)));

	m_free_const_enum.reserve(
		free_ids.size() + const_ids.size());

	for (auto id : free_ids)
		m_free_const_enum.emplace_back(id,
			SyntaxNodeType::FREE);

	for (auto id : const_ids)
		m_free_const_enum.emplace_back(id,
			SyntaxNodeType::CONSTANT);

	// now construct m_rule_iter
	restore_invariant();
}


bool SubExprMatchingIterator::valid() const
{
	return m_sub_expr_iter != m_forefront_stmt.end();
}


ProofStatePtr SubExprMatchingIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iter != nullptr);
	ATP_LOGIC_ASSERT(m_rule_iter->valid());

	return m_rule_iter->get();
}


void SubExprMatchingIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iter != nullptr);

	m_rule_iter->advance();

	if (!m_rule_iter->valid())
	{
		m_rule_iter.reset();
		++m_sub_expr_iter;
		if (valid())
		{
			restore_invariant();
		}
	}
}


size_t SubExprMatchingIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iter != nullptr);
	return m_rule_iter->size();
}


void SubExprMatchingIterator::restore_invariant()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_PRECOND(m_rule_iter == nullptr);

	m_rule_iter = RuleMatchingIterator::construct(m_ctx, m_ker,
		m_target_stmt, m_forefront_stmt, m_sub_expr_iter,
		m_free_const_enum);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


