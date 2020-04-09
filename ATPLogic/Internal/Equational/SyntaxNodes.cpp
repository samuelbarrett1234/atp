/**

\file

\author Samuel Barrett

*/


#include "SyntaxNodes.h"
#include <map>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include "ParseTreeFold.h"
#include "KnowledgeKernel.h"


namespace atp
{
namespace logic
{
namespace equational
{


SyntaxNodePtr ptree_to_stree(ParseNodePtr ptree,
	const KnowledgeKernel& ker)
{
	std::map<std::string, size_t> free_var_ids;
	size_t next_free_id = 0;
	size_t num_eq_signs_encountered = 0;

	return fold_parse_tree<SyntaxNodePtr>(
		[&num_eq_signs_encountered](SyntaxNodePtr lhs, SyntaxNodePtr rhs)
		-> SyntaxNodePtr
		{
			num_eq_signs_encountered++;

			// check for errors:
			if (lhs == nullptr || rhs == nullptr ||
				num_eq_signs_encountered > 1)
				return SyntaxNodePtr();

			return std::make_shared<EqSyntaxNode>(lhs, rhs);
		},
		[&free_var_ids, &next_free_id, &ker](std::string name,
			std::list<SyntaxNodePtr>::iterator child_begin,
			std::list<SyntaxNodePtr>::iterator child_end)
			-> SyntaxNodePtr
		{
			const auto arity = std::distance(child_begin, child_end);
			const bool identifier_defined = ker.is_defined(name);

			if (!identifier_defined)  // if a free variable
			{
				// free functions are not allowed!
				if (arity > 0)
					return SyntaxNodePtr();

				// construct free variable ID:
				auto name_id_iter = free_var_ids.find(name);
				if (name_id_iter == free_var_ids.end())
				{
					// encountered a new free variable:
					free_var_ids[name] = next_free_id;
					next_free_id++;
					return std::make_shared<FreeSyntaxNode>(
						next_free_id - 1
						);
				}
				else
				{
					// reuse ID from old free variable:
					return std::make_shared<FreeSyntaxNode>(
						name_id_iter->second
						);
				}
			}
			else if (arity == 0)  // if a user defined constant...
			{
				ATP_LOGIC_ASSERT(identifier_defined);

				// first check that we got the correct arity:
				if (ker.symbol_arity_from_name(name) != arity)
					return SyntaxNodePtr();

				const size_t symbol_id = ker.symbol_id(name);

				return std::make_shared<ConstantSyntaxNode>(
					symbol_id);
			}
			else  // if a user defined function...
			{
				ATP_LOGIC_ASSERT(identifier_defined && arity > 0);

				// first check that we got the correct arity:
				if (ker.symbol_arity_from_name(name) != arity)
					return SyntaxNodePtr();

				// if any child fails, we fail:
				if (std::any_of(child_begin, child_end,
					boost::phoenix::arg_names::arg1 == nullptr))
					return SyntaxNodePtr();

				const size_t symbol_id = ker.symbol_id(name);

				return std::make_shared<FuncSyntaxNode>(
					symbol_id, child_begin, child_end);
			}
		},
		ptree
	);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


