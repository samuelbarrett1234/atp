#include "EquationalLanguage.h"
#include "EquationalParser.h"
#include "EquationalSyntaxNodes.h"
#include <type_traits>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


bool EquationalLanguage::load_kernel(IKnowledgeKernel& _ker,
	std::istream& in) const
{
	// check they've given us a valid kernel:

	auto p_ker = dynamic_cast<EquationalKnowledgeKernel*>(
		&_ker
		);

	ATP_LOGIC_PRECOND(p_ker != nullptr);

	// parse the definitions, returning (maybe) a list of
	// (symbol name, symbol arity) pairs:
	auto result = parse_definitions(in);

	// if parse was successful...
	if (result.has_value())
	{
		// define each value in the kernel:
		for (auto pair : result.get())
		{
			p_ker->define_symbol(pair.first, pair.second);
		}

		return true;
	}
	else return false;
}


KnowledgeKernelPtr EquationalLanguage::create_empty_kernel() const
{
	return std::make_shared<EquationalKnowledgeKernel>();
}


StatementArrayPtr EquationalLanguage::create_stmts(std::istream& in,
	StmtFormat input_format, const IKnowledgeKernel& _ker) const
{
	const EquationalKnowledgeKernel* p_ker =
		dynamic_cast<const EquationalKnowledgeKernel*>(&_ker);
	ATP_LOGIC_PRECOND(p_ker != nullptr);

	switch (input_format)
	{
	case StmtFormat::TEXT:
	{
		// firstly, parse_statements the input text:
		auto parse_nodes = parse_statements(in);

		if (!parse_nodes.has_value())
			return StatementArrayPtr();  // failed due to parse_statements error

		std::vector<SyntaxNodePtr> syntax_nodes;
		syntax_nodes.reserve(parse_nodes.get().size());

		// now turn these into a more structured "syntax tree",
		// which also does type checking:
		std::transform(parse_nodes.get().begin(),
			parse_nodes.get().end(), std::back_inserter(
				syntax_nodes
			), boost::bind(&ptree_to_stree, _1,
				boost::ref(*p_ker)));

		if (std::any_of(syntax_nodes.begin(),
			syntax_nodes.end(),
			std::is_null_pointer<SyntaxNodePtr>()))
			return StatementArrayPtr();  // failed due to syntax error

		// now convert these into the final type

		EquationalStatementArray::ArrPtr pStmtArr =
			std::make_shared<EquationalStatementArray::ArrType>();
		pStmtArr->reserve(syntax_nodes.size());

		// construct EquationalStatementArray objects from the syntax
		// nodes:
		std::transform(syntax_nodes.begin(), syntax_nodes.end(),
			std::back_inserter(*pStmtArr),
			boost::phoenix::construct<EquationalStatementArray>(
				*p_ker, boost::phoenix::arg_names::arg1
				));

		return std::make_shared<EquationalStatementArray>(pStmtArr);
	}
	case StmtFormat::BINARY:
		return StatementArrayPtr();  // not implemented yet!!
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


