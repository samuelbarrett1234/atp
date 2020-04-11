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
#include "KnowledgeKernel.h"
#include "SemanticsHelper.h"


namespace atp
{
namespace logic
{
namespace equational
{


Statement::Statement(
	const KnowledgeKernel& ker,
	SyntaxNodePtr p_root) :
	m_ker(ker)
{
	ATP_LOGIC_PRECOND(p_root->get_type() ==
		SyntaxNodeType::EQ);

	typedef std::pair<size_t, SyntaxNodeType> NodePair;

	auto eq_func = [this](NodePair l, NodePair r)
		-> NodePair
	{
		m_left = l.first;
		m_left_type = l.second;
		m_right = r.first;
		m_right_type = r.second;
		
		// return bogus value
		return std::make_pair(0, SyntaxNodeType::EQ);
	};

	auto free_func = [this](size_t free_id) -> NodePair
	{
		m_free_var_ids.insert(free_id);
		return std::make_pair(free_id, SyntaxNodeType::FREE);
	};

	auto const_func = [](size_t symb_id) -> NodePair
	{
		return std::make_pair(symb_id, SyntaxNodeType::CONSTANT);
	};

	auto f_func = [this](size_t symb_id,
		std::vector<NodePair>::iterator begin,
		std::vector<NodePair>::iterator end) -> NodePair
	{
		// when we add a new element to all the vectors, the index
		// will be the newsize-1, which is just the oldsize.
		const size_t func_idx = m_func_children.size();

		const size_t arity = std::distance(begin, end);

		// arity limit!!!
		ATP_LOGIC_PRECOND(arity < MAX_ARITY);

		m_func_arity.push_back(arity);
		m_func_symb_ids.push_back(symb_id);

		auto map_first = boost::bind(&NodePair::first, _1);
		auto map_second = boost::bind(&NodePair::second, _1);

		m_func_children.emplace_back();
		m_func_child_types.emplace_back();

		std::copy(boost::make_transform_iterator(begin, map_first),
			boost::make_transform_iterator(end, map_first),
			m_func_children.back().begin());

		std::copy(boost::make_transform_iterator(begin, map_second),
			boost::make_transform_iterator(end, map_second),
			m_func_child_types.back().begin());

		return std::make_pair(func_idx, SyntaxNodeType::FUNC);
	};

	// ignore return value
	fold_syntax_tree<NodePair>(eq_func, free_func, const_func,
		f_func, p_root);
}


Statement::Statement(const Statement& other) :
	m_ker(other.m_ker)
{
	*this = other;
}


Statement::Statement(Statement&& other) noexcept :
	m_ker(other.m_ker)
{
	*this = std::move(other);
}


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_left = other.m_left;
		m_left_type = other.m_left_type;
		m_right = other.m_right;
		m_right_type = other.m_right_type;
		m_func_symb_ids = other.m_func_symb_ids;
		m_func_arity = other.m_func_arity;
		m_func_children = other.m_func_children;
		m_func_child_types = other.m_func_child_types;
		m_free_var_ids = other.m_free_var_ids;
	}
	return *this;
}


Statement& Statement::operator=(Statement&& other) noexcept
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_left = std::move(other.m_left);
		m_left_type = std::move(other.m_left_type);
		m_right = std::move(other.m_right);
		m_right_type = std::move(other.m_right_type);
		m_func_symb_ids = std::move(other.m_func_symb_ids);
		m_func_arity = std::move(other.m_func_arity);
		m_func_children = std::move(other.m_func_children);
		m_func_child_types = std::move(other.m_func_child_types);
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
		return m_ker.symbol_name(symb_id);
	};
	auto func_fold = [this](size_t symb_id,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) -> std::string
	{
		return m_ker.symbol_name(symb_id) + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ") + ')';
	};

	return fold<std::string>(eq_fold, free_var_fold,
		const_fold, func_fold);
}


Statement Statement::transpose() const
{
	Statement tr = *this;

	std::swap(tr.m_left, tr.m_right);
	std::swap(tr.m_left_type, tr.m_right_type);

	return tr;
}


std::pair<SyntaxNodePtr, SyntaxNodePtr> Statement::get_sides() const
{
	return std::make_pair(to_syntax_tree(m_left, m_left_type),
		to_syntax_tree(m_right, m_right_type));
}


Statement Statement::adjoin_rhs(const Statement& other) const
{
	ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);

	// \todo: this could be more efficient I think, but it would
	// be a considerable amount of effort (perhaps a couple of
	// hundred lines?)

	auto my_sides = get_sides();
	auto other_sides = other.get_sides();

	auto new_eq = EqSyntaxNode::construct(my_sides.first,
		other_sides.second);

	return Statement(m_ker, new_eq);
}


SyntaxNodePtr Statement::to_syntax_tree(size_t idx,
	SyntaxNodeType type) const
{
	ATP_LOGIC_PRECOND(type != SyntaxNodeType::EQ);

	switch (type)
	{
	case SyntaxNodeType::FREE:
		return FreeSyntaxNode::construct(idx);
	case SyntaxNodeType::CONSTANT:
		return ConstantSyntaxNode::construct(idx);
	case SyntaxNodeType::FUNC:
	{
		// eek recursion
		// \todo use a stack (a fold-like operation) here

		std::vector<SyntaxNodePtr> children;
		children.reserve(m_func_arity[idx]);

		for (size_t i = 0; i < m_func_arity[idx]; ++i)
		{
			children.push_back(to_syntax_tree(
				m_func_children[idx][i],
				m_func_child_types[idx][i]));
		}

		return FuncSyntaxNode::construct(m_func_symb_ids[idx],
			children.begin(), children.end());
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid type");
		return SyntaxNodePtr();
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


