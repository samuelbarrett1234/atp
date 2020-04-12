/**

\file

\author Samuel Barrett

*/


#include "Statement.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include "SyntaxTreeFold.h"
#include "ModelContext.h"
#include "SemanticsHelper.h"


namespace phx = boost::phoenix;


namespace atp
{
namespace logic
{
namespace equational
{


Statement::Statement(
	const ModelContext& ctx,
	const SyntaxNodePtr& p_root) :
	m_ctx(ctx)
{
	ATP_LOGIC_PRECOND(p_root->get_type() ==
		SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(p_root.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	m_sides = std::make_pair(Expression::construct(ctx,
		p_eq->left()), Expression::construct(ctx,
			p_eq->right()));

	m_tree_sides = std::make_pair(p_eq->left(), p_eq->right());

	// collect together the free variable IDs by unioning those
	// of the children:

	const auto& first_ids = m_sides.first->free_var_ids();
	const auto& second_ids = m_sides.second->free_var_ids();

	std::set_union(first_ids.begin(), first_ids.end(),
		second_ids.begin(), second_ids.end(),
		std::inserter(m_free_var_ids,
			m_free_var_ids.end()));
}


Statement::Statement(const Statement& other) :
	m_ctx(other.m_ctx)
{
	*this = other;
}


Statement::Statement(Statement&& other) noexcept :
	m_ctx(other.m_ctx)
{
	*this = std::move(other);
}


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
		m_sides = other.m_sides;
		m_tree_sides = other.m_tree_sides;
		m_free_var_ids = other.m_free_var_ids;
	}
	return *this;
}


Statement& Statement::operator=(Statement&& other) noexcept
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);
		m_sides = std::move(other.m_sides);
		m_tree_sides = std::move(other.m_tree_sides);
		m_free_var_ids = std::move(other.m_free_var_ids);
	}
	return *this;
}


std::string Statement::to_str() const
{
	// this is a fold!

	auto eq_fold = [](std::string lhs, std::string rhs)
		-> std::string
	{
		return lhs + " = " + rhs;
	};
	auto free_var_fold = [](size_t free_var_id) -> std::string
	{
		return "x" + boost::lexical_cast<std::string>(free_var_id);
	};
	auto const_fold = [this](size_t symb_id) -> std::string
	{
		return m_ctx.symbol_name(symb_id);
	};
	auto func_fold = [this](size_t symb_id,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) -> std::string
	{
		return m_ctx.symbol_name(symb_id) + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ") + ')';
	};

	return fold<std::string>(eq_fold, free_var_fold,
		const_fold, func_fold);
}


Statement Statement::transpose() const
{
	Statement tr = *this;

	std::swap(tr.m_sides.first, tr.m_sides.second);

	return tr;
}


std::pair<SyntaxNodePtr, SyntaxNodePtr> Statement::get_sides() const
{
	//auto to_syntax_tree = [this](const Expression& expr)
	//	-> SyntaxNodePtr
	//{
	//	return expr.fold<SyntaxNodePtr>(&FreeSyntaxNode::construct,
	//		&ConstantSyntaxNode::construct,
	//		&FuncSyntaxNode::construct);
	//};

	//return std::make_pair(to_syntax_tree(*m_sides.first),
	//	to_syntax_tree(*m_sides.second));

	return m_tree_sides;
}


Statement Statement::adjoin_rhs(const Statement& other) const
{
	ATP_LOGIC_PRECOND(&m_ctx == &other.m_ctx);

	// \todo: this could be more efficient I think, but it would
	// be a considerable amount of effort (perhaps a couple of
	// hundred lines?)

	auto my_sides = get_sides();
	auto other_sides = other.get_sides();

	auto new_eq = EqSyntaxNode::construct(my_sides.first,
		other_sides.second);

	return Statement(m_ctx, new_eq);
}


Statement Statement::map_free_vars(const std::map<size_t,
	SyntaxNodePtr> free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(m_free_var_ids.begin(),
		m_free_var_ids.end(),
		[&free_map](size_t id)
		{ return free_map.find(id) != free_map.end(); }));

	Statement new_stmt = *this;

	// create a separate copy
	new_stmt.m_sides.first = std::make_shared<Expression>(
		*m_sides.first);
	new_stmt.m_sides.second = std::make_shared<Expression>(
		*m_sides.second);

	*new_stmt.m_sides.first = m_sides.first->map_free_vars(
		free_map);
	*new_stmt.m_sides.second = m_sides.second->map_free_vars(
		free_map);

	// clear this and rebuild from the union of both children
	new_stmt.m_free_var_ids.clear();

	const auto& first_ids = new_stmt.m_sides.first->free_var_ids();
	const auto& second_ids = new_stmt.m_sides.second->free_var_ids();
	
	std::set_union(first_ids.begin(), first_ids.end(),
		second_ids.begin(), second_ids.end(),
		std::inserter(new_stmt.m_free_var_ids,
			new_stmt.m_free_var_ids.end()));

	return new_stmt;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


