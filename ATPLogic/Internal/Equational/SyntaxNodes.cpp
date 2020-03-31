#include "SyntaxNodes.h"
#include "ParseTreeFold.h"
#include <boost/bind.hpp>
#include <map>


namespace atp
{
namespace logic
{
namespace equational
{


SyntaxNodePtr ptree_to_stree(ParseNodePtr ptree,
	const EquationalKnowledgeKernel& ker)
{
	std::map<std::string, size_t> free_var_ids;
	size_t next_free_id = 0;
	size_t num_eq_signs_encountered = 0;

	return fold_parse_tree(
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

			// check that we have no free variables with nonzero
			// arity:
			ATP_LOGIC_PRECOND(arity == 0 || identifier_defined);

			if (!identifier_defined)  // if a free variable
			{
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
				const size_t symbol_id = ker.symbol_id(name);

				return std::make_shared<ConstantSyntaxNode>(
					symbol_id);
			}
			else  // if a user defined function...
			{
				ATP_LOGIC_ASSERT(identifier_defined && arity > 0);

				// if any child fails, we fail:
				if (std::any_of(child_begin, child_end,
					std::is_null_pointer<SyntaxNodePtr>()))
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


