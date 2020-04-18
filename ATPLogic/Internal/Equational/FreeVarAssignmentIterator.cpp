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
	FreeVarIdSet::const_iterator remaining_free_end)
{
	return std::make_shared<FreeVarAssignmentIterator>(ctx, ker,
		parent, std::move(subbed_stmt), free_const_enum,
		remaining_free_begin, remaining_free_end);
}


FreeVarAssignmentIterator::FreeVarAssignmentIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent, Statement subbed_stmt,
	const std::vector<std::pair<size_t,
		SyntaxNodeType>>& free_const_enum,
	FreeVarIdSet::const_iterator remaining_free_begin,
	FreeVarIdSet::const_iterator remaining_free_end) :
	m_ctx(ctx), m_ker(ker), m_parent(parent),
	m_subbed_stmt(std::move(subbed_stmt)),
	m_free_const_enum(free_const_enum),
	m_remaining_free_begin(remaining_free_begin),
	m_remaining_free_end(remaining_free_end),
	m_current_free_value(free_const_enum.begin()),
	m_is_leaf(remaining_free_begin == remaining_free_end),
	m_leaf_is_done(false)
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
		const bool is_valid =
			(m_current_free_value != m_free_const_enum.end());

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
			++m_current_free_value;
			m_child.reset();

			if (m_current_free_value != m_free_const_enum.end())
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
	ATP_LOGIC_PRECOND(m_current_free_value !=
		m_free_const_enum.end());

	ATP_LOGIC_ASSERT(m_remaining_free_begin !=
		m_remaining_free_end);

	// make our current substitution to get a new statement
	// which we will pass to the child we are about to construct
	Statement child_stmt = [this]() -> Statement
	{
		switch (m_current_free_value->second)
		{
		case SyntaxNodeType::FREE:
			return m_subbed_stmt.replace_free_with_free(
				// the free variable ID beforehand:
				*m_remaining_free_begin,
				// the new free variable ID:
				m_current_free_value->first);
		case SyntaxNodeType::CONSTANT:
			return m_subbed_stmt.replace_free_with_const(
				// the free variable ID beforehand:
				*m_remaining_free_begin,
				// the constant we are replacing it with:
				m_current_free_value->first);
		default:
			ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
			throw std::exception();
		}
	}();

	m_child = FreeVarAssignmentIterator::construct(m_ctx, m_ker,
		m_parent, std::move(child_stmt), m_free_const_enum,
		std::next(m_remaining_free_begin), m_remaining_free_end);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


