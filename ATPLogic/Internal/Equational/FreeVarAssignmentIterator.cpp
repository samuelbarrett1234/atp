/**

\file

\author Samuel Barrett

*/


#include "FreeVarAssignmentIterator.h"
#include "Expression.h"
#include "Statement.h"
#include "ModelContext.h"
#include "KnowledgeKernel.h"
#include "ProofState.h"


namespace atp
{
namespace logic
{
namespace equational
{


std::shared_ptr<FreeVarAssignmentIterator> FreeVarAssignmentIterator::construct(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent, Statement subbed_stmt,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum,
	FreeVarIdSet::const_iterator remaining_free_begin,
	FreeVarIdSet::const_iterator remaining_free_end,
	bool randomised)
{
	return std::make_shared<FreeVarAssignmentIterator>(ctx, ker,
		parent, std::move(subbed_stmt), free_const_enum,
		remaining_free_begin, remaining_free_end, randomised);
}


FreeVarAssignmentIterator::FreeVarAssignmentIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent, Statement subbed_stmt,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum,
	FreeVarIdSet::const_iterator remaining_free_begin,
	FreeVarIdSet::const_iterator remaining_free_end,
	bool randomised) :
	m_ctx(ctx), m_ker(ker), m_parent(parent),
	m_subbed_stmt(std::move(subbed_stmt)),
	m_free_const_enum(free_const_enum),
	m_remaining_free_begin(remaining_free_begin),
	m_remaining_free_end(remaining_free_end),
	m_cur_free_idx(free_const_enum.size(), randomised,
		ker.generate_rand()),
	m_is_leaf(remaining_free_begin == remaining_free_end),
	m_leaf_is_done(false), m_randomised(randomised)
{
	if (!m_is_leaf)
		// build m_child for non-leaves
		restore_invariant();
	else
		// load the leaf result once for the lifetime of this iter
		m_leaf_result = std::make_shared<ProofState>(
			m_parent, m_subbed_stmt);
}


bool FreeVarAssignmentIterator::valid() const
{
	if (m_is_leaf)
	{
		return !m_leaf_is_done;
	}
	else
	{
		const bool is_valid = !m_cur_free_idx.is_end();

		// check the invariant
		ATP_LOGIC_ASSERT(!is_valid
			|| (m_child != nullptr && m_child->valid()));

		return is_valid;
	}
}


ProofStatePtr FreeVarAssignmentIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());

	if (!m_is_leaf)
	{
		ATP_LOGIC_ASSERT(m_child != nullptr);
		ATP_LOGIC_ASSERT(m_child->valid());

		return m_child->get();
	}
	else
	{
		return m_leaf_result;
	}
}


void FreeVarAssignmentIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());

	if (!m_is_leaf)
	{
		// delegate

		ATP_LOGIC_ASSERT(m_child != nullptr);
		ATP_LOGIC_ASSERT(m_child->valid());

		m_child->advance();

		if (!m_child->valid())
		{
			m_cur_free_idx.advance();
			m_child.reset();

			if (!m_cur_free_idx.is_end())
			{
				restore_invariant();
			}
		}
	}
	else
	{
		m_leaf_is_done = true;
		m_leaf_result.reset();
	}
}


size_t FreeVarAssignmentIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());

	if (m_is_leaf)
	{
		return 1;
	}
	else
	{
		ATP_LOGIC_ASSERT(m_child != nullptr);
		ATP_LOGIC_ASSERT(m_child->valid());
		return 1 + m_child->size();
	}
}


void FreeVarAssignmentIterator::restore_invariant()
{
	ATP_LOGIC_PRECOND(!m_is_leaf);
	ATP_LOGIC_PRECOND(m_child == nullptr);

	// check valid() without actually calling valid() because
	// otherwise the assertions will fail.
	ATP_LOGIC_PRECOND(!m_cur_free_idx.is_end());
	ATP_LOGIC_ASSERT(m_cur_free_idx.get() <
		m_free_const_enum.size());

	ATP_LOGIC_ASSERT(m_remaining_free_begin !=
		m_remaining_free_end);

	// make our current substitution to get a new statement
	// which we will pass to the child we are about to construct
	Statement child_stmt = [this]() -> Statement
	{
		// get ID and type under current index

		const size_t free_const_id = m_free_const_enum[
			m_cur_free_idx.get()].first;
		const SyntaxNodeType free_const_type = m_free_const_enum[
			m_cur_free_idx.get()].second;

		switch (free_const_type)
		{
		case SyntaxNodeType::FREE:
			return m_subbed_stmt.replace_free_with_free(
				// the free variable ID beforehand:
				*m_remaining_free_begin,
				// the new free variable ID:
				free_const_id);
		case SyntaxNodeType::CONSTANT:
			return m_subbed_stmt.replace_free_with_const(
				// the free variable ID beforehand:
				*m_remaining_free_begin,
				// the constant we are replacing it with:
				free_const_id);
		default:
			ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
			throw std::exception();
		}
	}();

	m_child = FreeVarAssignmentIterator::construct(m_ctx, m_ker,
		m_parent, std::move(child_stmt), m_free_const_enum,
		std::next(m_remaining_free_begin), m_remaining_free_end,
		m_randomised);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


