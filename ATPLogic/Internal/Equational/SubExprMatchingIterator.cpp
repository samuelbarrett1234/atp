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
	m_randomised(randomised), m_cur_idx(0)
{
	// get all the sub expressions:
	auto stmt_iter = m_forefront_stmt.begin();
	while (stmt_iter != m_forefront_stmt.end())
	{
		m_sub_expr_iters.emplace_back(stmt_iter);
		++stmt_iter;
	}
	m_rule_iters.resize(m_sub_expr_iters.size());

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

	// construct the current rule iterator
	// (this will behave well if there doesn't exist a good
	// successor)
	if (m_randomised)
	{
		reset_current();
	}
	else
	{
		// need to handle this slightly differently if we are not in
		// random mode
		construct_child();
		if (!m_rule_iters[m_cur_idx]->valid())
			reset_current();
	}
}


bool SubExprMatchingIterator::valid() const
{
	const bool is_valid = m_cur_idx != m_sub_expr_iters.size();

	// check invariant while we're here
	ATP_LOGIC_ASSERT(!is_valid || (m_rule_iters[m_cur_idx] != nullptr
		&& m_rule_iters[m_cur_idx]->valid()));

	return is_valid;
}


ProofStatePtr SubExprMatchingIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iters[m_cur_idx] != nullptr);
	ATP_LOGIC_ASSERT(m_rule_iters[m_cur_idx]->valid());

	return m_rule_iters[m_cur_idx]->get();
}


void SubExprMatchingIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iters[m_cur_idx] != nullptr);
	ATP_LOGIC_ASSERT(m_rule_iters[m_cur_idx]->valid());

	m_rule_iters[m_cur_idx]->advance();

	// in non-randomised mode, only advance to next index when the
	// current rule is exhausted
	if (m_randomised || !m_rule_iters[m_cur_idx]->valid())
		reset_current();
}


size_t SubExprMatchingIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_rule_iters[m_cur_idx] != nullptr);
	ATP_LOGIC_ASSERT(m_rule_iters[m_cur_idx]->valid());
	return m_rule_iters[m_cur_idx]->size();
}


void SubExprMatchingIterator::construct_child()
{
	ATP_LOGIC_PRECOND(m_rule_iters[m_cur_idx] == nullptr);

	// don't forget to keep m_sub_expr_iters[m_cur_idx] around after
	// because m_rule_iters requires a reference to it!
	m_rule_iters[m_cur_idx] = RuleMatchingIterator::construct(m_ctx,
		m_ker, m_parent, m_forefront_stmt,
		m_sub_expr_iters[m_cur_idx], m_free_const_enum,
		m_randomised);
}


void SubExprMatchingIterator::reset_current()
{
	do
	{
		if (m_randomised)
		{
			// get random index
			m_cur_idx = m_ker.generate_rand() %
				m_sub_expr_iters.size();
		}
		else
		{
			// get next index
			++m_cur_idx;

			// if this happens, we immediately know we're done and
			// can exit this function totally
			if (m_cur_idx == m_rule_iters.size())
				return;
		}

		// if this index hasn't been encountered before...
		if (m_rule_iters[m_cur_idx] == nullptr)
			construct_child();
	}
	// while there exists a good rule, but the current one is invalid
	// (i.e. reached its end)
	while (!m_rule_iters[m_cur_idx]->valid() &&
		std::any_of(m_rule_iters.begin(), m_rule_iters.end(),
		[](const auto& p) { return p == nullptr || p->valid(); }));

	// if our current rule is invalid, the only way this could've
	// happened is if the above loop terminated because there were no
	// other good rules! Hence we are done
	if (!m_rule_iters[m_cur_idx]->valid())
	{
		m_cur_idx = m_sub_expr_iters.size();
		ATP_LOGIC_ASSERT(m_sub_expr_iters.size() == m_rule_iters.size());
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


