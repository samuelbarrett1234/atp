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
	StmtFormat input_format, const IModelContext& _ctx) const
{
	const ModelContext* p_ctx =
		dynamic_cast<const ModelContext*>(&_ctx);
	ATP_LOGIC_PRECOND(p_ctx != nullptr);

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
				boost::ref(*p_ctx)));

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
			pStmtArr->emplace_back(*p_ctx, *iter);
		}

		return std::make_shared<StatementArray>(pStmtArr);
	}
	case StmtFormat::BINARY:
		return std::make_shared<StatementArray>(
			StatementArray::load_from_bin(*p_ctx, in));
	default:
		ATP_LOGIC_PRECOND(false && "invalid statement type!");
		return StatementArrayPtr();
	}
}


void Language::serialise_stmts(std::ostream& out,
	const StatementArrayPtr& _p_stmts,
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
		p_stmts->save(out);
		break;
	default:
		ATP_LOGIC_PRECOND(false && "invalid statement type!");
		break;
	}
}


StatementArrayPtr Language::normalise(
	const StatementArrayPtr& _p_stmts)
{
	// convert to derived type:
	auto p_stmts = dynamic_cast<const StatementArray*>(
		_p_stmts.get()
		);

	ATP_LOGIC_PRECOND(p_stmts != nullptr);

	std::vector<Statement> new_arr;
	new_arr.reserve(p_stmts->size());

	for (auto stmt : *p_stmts)
	{
		// if not equivalent to any of the statements we have already
		// added
		if (std::none_of(new_arr.begin(),
			new_arr.end(), boost::bind(&Statement::equivalent, _1,
				boost::ref(stmt))))
		{
			// now we need to force an order about the equals sign:
			if (stmt.transpose().to_str() > stmt.to_str())
			{
				stmt = stmt.transpose();
			}

			// now we need to reduce the free variable IDs:
			auto ids = stmt.free_var_ids();
			FreeVarMap<Expression> map;
			
			// build a map which reduces the IDs to be a contiguous
			// block starting at 0
			{
				size_t i = 0;
				auto iter = ids.begin();
				while (iter != ids.end())
				{
					map.insert(*iter,
						Expression(stmt.context(), i,
							SyntaxNodeType::FREE));

					++i;
					++iter;
				}
			}

			stmt = stmt.map_free_vars(map);

			// now we're ready to add it:
			new_arr.emplace_back(std::move(stmt));
		}
	}

	return std::make_shared<StatementArray>(
		std::make_shared<StatementArray::ArrType>(
			std::move(new_arr)));
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


