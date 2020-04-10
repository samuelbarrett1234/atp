/**

\file

\author Samuel Barrett

*/


#include "Statement.h"
#include <set>
#include <list>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/iterator/zip_iterator.hpp>
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
	m_ker(ker), m_root(p_root),
	m_free_var_ids(semantics::get_free_var_ids(p_root))
{
	ATP_LOGIC_PRECOND(m_root != nullptr);
	ATP_LOGIC_PRECOND(m_root->get_type() == SyntaxNodeType::EQ);
}


Statement::Statement(const Statement& other) :
	m_ker(other.m_ker),
	m_root(other.m_root),
	m_free_var_ids(other.m_free_var_ids)
{ }


Statement::Statement(Statement&& other) noexcept :
	m_ker(other.m_ker),
	m_root(std::move(other.m_root)),
	m_free_var_ids(other.m_free_var_ids)
{ }


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_root = other.m_root;
		m_free_var_ids = other.m_free_var_ids;
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
	auto p_eq = dynamic_cast<const EqSyntaxNode*>(m_root.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	auto p_new_eq = EqSyntaxNode::construct(p_eq->right(),
		p_eq->left());

	return Statement(m_ker, p_new_eq);
}


std::pair<SyntaxNodePtr, SyntaxNodePtr> Statement::get_sides() const
{
	auto p_eq = dynamic_cast<const EqSyntaxNode*>(m_root.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	return std::make_pair(p_eq->left(), p_eq->right());
}


Statement Statement::adjoin_rhs(const Statement& other) const
{
	ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(m_root.get());
	auto p_other_eq = dynamic_cast<EqSyntaxNode*>(
		other.m_root.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);
	ATP_LOGIC_ASSERT(p_other_eq != nullptr);

	auto p_new_eq = EqSyntaxNode::construct(p_eq->left(),
		p_other_eq->right());

	return Statement(m_ker, p_new_eq);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


