/**

\file

\author Samuel Barrett

*/

#include "Language.h"
#include "Parser.h"
#include "SyntaxNodes.h"
#include "KnowledgeKernel.h"
#include "StatementArray.h"
#include "ModelContext.h"
#include <boost/phoenix.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/bind.hpp>


namespace atp
{
namespace logic
{
namespace equational
{


ModelContextPtr Language::try_create_context(
	std::istream& in) const
{
	return ModelContext::try_construct(*this, in);
}


KnowledgeKernelPtr Language::try_create_kernel(
	const IModelContext& _ctx) const
{
	const ModelContext* p_ctx = dynamic_cast<
		const ModelContext*>(&_ctx);

	// check type
	if (p_ctx == nullptr)
		return KnowledgeKernelPtr();

	return KnowledgeKernel::try_construct(*this, *p_ctx);
}


StatementArrayPtr Language::deserialise_stmts(std::istream& in,
	StmtFormat input_format, const IKnowledgeKernel& _ker) const
{
	const KnowledgeKernel* p_ker =
		dynamic_cast<const KnowledgeKernel*>(&_ker);
	ATP_LOGIC_PRECOND(p_ker != nullptr);

	switch (input_format)
	{
	case StmtFormat::TEXT:
	{
		// firstly, parse_statements the input text:
		auto parse_nodes = parse_statements(in);

		if (!parse_nodes.has_value())
			// failed due to parse_statements error
			return StatementArrayPtr();

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
			boost::phoenix::arg_names::arg1 == nullptr))
			// failed due to syntax error
			return StatementArrayPtr();

		// now convert these into the final type

		StatementArray::ArrPtr pStmtArr =
			std::make_shared<StatementArray::ArrType>();
		pStmtArr->reserve(syntax_nodes.size());

		// construct StatementArray objects from the syntax
		// nodes:
		for (auto iter = syntax_nodes.begin();
			iter != syntax_nodes.end(); ++iter)
		{
			pStmtArr->emplace_back(*p_ker, *iter);
		}

		return std::make_shared<StatementArray>(pStmtArr);
	}
	case StmtFormat::BINARY:
		return StatementArrayPtr();  // not implemented yet!!
	default:
		ATP_LOGIC_PRECOND(false && "invalid statement type!");
		return StatementArrayPtr();
	}
}


void Language::serialise_stmts(std::ostream& out,
	StatementArrayPtr _p_stmts,
	StmtFormat output_format) const
{
	// convert to derived type:
	auto p_stmts = dynamic_cast<const StatementArray*>(
		_p_stmts.get()
		);

	ATP_LOGIC_PRECOND(p_stmts != nullptr);

	switch (output_format)
	{
	case StmtFormat::TEXT:
	{
		std::vector<std::string> as_strs;
		as_strs.reserve(p_stmts->size());

		// convert each statement to string
		std::transform(p_stmts->begin(), p_stmts->end(),
			std::back_inserter(as_strs),
			boost::bind(&Statement::to_str, _1));

		// output the statements separated by newlines
		out << boost::algorithm::join(as_strs, "\n");
	}
		break;
	case StmtFormat::BINARY:
		break;  // not implemented yet!!
	default:
		ATP_LOGIC_PRECOND(false && "invalid statement type!");
		break;
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


