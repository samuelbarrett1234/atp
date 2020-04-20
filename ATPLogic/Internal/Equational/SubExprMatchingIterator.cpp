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


std::shared_ptr<SubExprMatchingIterator> SubExprMatchingIterator::construct(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent, const Statement& forefront_stmt,
	bool randomised)
{
	return std::make_shared<SubExprMatchingIterator>(ctx, ker,
		parent, forefront_stmt, randomised);
}


SubExprMatchingIterator::SubExprMatchingIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent, const Statement& forefront_stmt,
	bool randomised) :
	m_ctx(ctx), m_ker(ker), m_parent(parent),
	m_forefront_stmt(forefront_stmt),
	m_sub_expr_iter(forefront_stmt.begin()),
	m_randomised(randomised)
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
	construct_child();

	// handle this special case (where lots of the matching rules
	// are invalid)
	while (m_sub_expr_iter != m_forefront_stmt.end()
		&& !m_rule_iter->valid())
	{
		m_rule_iter.reset();
		++m_sub_expr_iter;
		if (m_sub_expr_iter != m_forefront_stmt.end())
		{
			construct_child();
		}
	}
}


bool SubExprMatchingIterator::valid() const
{
	const bool is_valid = m_sub_expr_iter != m_forefront_stmt.end();

	// check invariant while we're here
	ATP_LOGIC_ASSERT(!is_valid || (m_rule_iter != nullptr
		&& m_rule_iter->valid()));

	return is_valid;
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

	while (m_sub_expr_iter != m_forefront_stmt.end()
		&& !m_rule_iter->valid())
	{
		m_rule_iter.reset();
		++m_sub_expr_iter;
		if (m_sub_expr_iter != m_forefront_stmt.end())
		{
			construct_child();
		}
	}
}


size_t SubExprMatchingIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iter != nullptr);
	return m_rule_iter->size();
}


void SubExprMatchingIterator::construct_child()
{
	ATP_LOGIC_PRECOND(m_sub_expr_iter != m_forefront_stmt.end());
	ATP_LOGIC_PRECOND(m_rule_iter == nullptr);

	m_rule_iter = RuleMatchingIterator::construct(m_ctx, m_ker,
		m_parent, m_forefront_stmt, m_sub_expr_iter,
		m_free_const_enum, m_randomised);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


