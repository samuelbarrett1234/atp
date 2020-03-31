#include "Statement.h"
#include "Matching.h"
#include "SyntaxTreeFold.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/range.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


EquationalStatement::EquationalStatement(
	EquationalKnowledgeKernel& ker,
	SyntaxNodePtr p_root) :
	m_ker(ker), m_root(p_root)
{
	ATP_LOGIC_PRECOND(m_root != nullptr);
	ATP_LOGIC_PRECOND(
		!equational::needs_free_var_id_rebuild(m_root));
}


StmtForm EquationalStatement::form() const
{
	if (equational::trivially_true(*m_root))
	{
		return StmtForm::CANONICAL_TRUE;
	}
	else return StmtForm::NOT_CANONICAL;

	// equational statements cannot be canonically false
}


std::string EquationalStatement::to_str() const
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
		return m_ker.symbol_name(symb_id) +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ");
	};

	return fold_syntax_tree(eq_fold, free_var_fold,
		const_fold, func_fold, m_root);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


