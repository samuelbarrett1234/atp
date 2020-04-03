#include "Statement.h"
#include "SyntaxTreeFold.h"
#include "KnowledgeKernel.h"
#include <set>
#include <list>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


std::set<size_t> get_free_var_ids(SyntaxNodePtr p_node);


Statement::Statement(
	const KnowledgeKernel& ker,
	SyntaxNodePtr p_root) :
	m_ker(ker), m_root(p_root),
	m_num_free_vars(get_free_var_ids(p_root).size())
{
	ATP_LOGIC_PRECOND(m_root != nullptr);
	ATP_LOGIC_PRECOND(m_root->get_type() == SyntaxNodeType::EQ);
}


Statement::Statement(const Statement& other) :
	m_ker(other.m_ker),
	m_root(other.m_root),
	m_num_free_vars(other.m_num_free_vars)
{ }


Statement::Statement(Statement&& other) noexcept :
	m_ker(other.m_ker),
	m_root(std::move(other.m_root)),
	m_num_free_vars(other.m_num_free_vars)
{ }


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_root = other.m_root;
		m_num_free_vars = other.m_num_free_vars;
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
		std::list<std::string>::iterator begin,
		std::list<std::string>::iterator end) -> std::string
	{
		return m_ker.symbol_name(symb_id) + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ") + ')';
	};

	return fold<std::string>(eq_fold, free_var_fold,
		const_fold, func_fold);
}


std::set<size_t> get_free_var_ids(SyntaxNodePtr p_node)
{
	std::list<SyntaxNodePtr> stack;
	std::set<size_t> var_ids;

	stack.push_back(p_node);

	while (!stack.empty())
	{
		p_node = stack.back();
		stack.pop_back();

		apply_to_syntax_node<void>(
			[&stack](EqSyntaxNode& node)
			{
				stack.push_back(node.left());
				stack.push_back(node.right());
			},
			[&var_ids](FreeSyntaxNode& node)
			{
				var_ids.insert(node.get_free_id());
			},
			[](ConstantSyntaxNode&) {},
			[&stack](FuncSyntaxNode& node)
			{
				stack.insert(stack.end(), node.begin(), node.end());
			},
			*p_node);
	}

	return var_ids;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


