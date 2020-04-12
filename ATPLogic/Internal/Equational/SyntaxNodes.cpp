/**

\file

\author Samuel Barrett

*/


#include "SyntaxNodes.h"
#include <map>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include <boost/pool/pool_alloc.hpp>
#include "ParseTreeFold.h"
#include "ModelContext.h"


namespace atp
{
namespace logic
{
namespace equational
{


SyntaxNodePtr ptree_to_stree(ParseNodePtr ptree,
	const ModelContext& ctx)
{
	std::map<std::string, size_t> free_var_ids;
	size_t next_free_id = 0;

	return fold_parse_tree<SyntaxNodePtr>(
		[](const SyntaxNodePtr& lhs, const SyntaxNodePtr& rhs)
		-> SyntaxNodePtr
		{
			// check for errors:
			if (lhs == nullptr || rhs == nullptr)
				return SyntaxNodePtr();

			return EqSyntaxNode::construct(lhs, rhs);
		},
		[&free_var_ids, &next_free_id, &ctx](std::string name,
			std::vector<SyntaxNodePtr>::iterator child_begin,
			std::vector<SyntaxNodePtr>::iterator child_end)
			-> SyntaxNodePtr
		{
			const auto arity = std::distance(child_begin, child_end);
			const bool identifier_defined = ctx.is_defined(name);

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
					return FreeSyntaxNode::construct(
						next_free_id - 1
						);
				}
				else
				{
					// reuse ID from old free variable:
					return FreeSyntaxNode::construct(
						name_id_iter->second
						);
				}
			}
			else if (arity == 0)  // if a user defined constant...
			{
				ATP_LOGIC_ASSERT(identifier_defined);

				// first check that we got the correct arity:
				if (ctx.symbol_arity(name) != arity)
					return SyntaxNodePtr();

				const size_t symbol_id = ctx.symbol_id(name);

				return ConstantSyntaxNode::construct(
					symbol_id);
			}
			else  // if a user defined function...
			{
				ATP_LOGIC_ASSERT(identifier_defined && arity > 0);

				// first check that we got the correct arity:
				if (ctx.symbol_arity(name) != arity)
					return SyntaxNodePtr();

				// if any child fails, we fail:
				if (std::any_of(child_begin, child_end,
					boost::phoenix::arg_names::arg1 == nullptr))
					return SyntaxNodePtr();

				const size_t symbol_id = ctx.symbol_id(name);

				return FuncSyntaxNode::move_construct(
					symbol_id, child_begin, child_end);
			}
		},
		ptree
	);
}


SyntaxNodePtr EqSyntaxNode::construct(const SyntaxNodePtr& lhs,
	const SyntaxNodePtr& rhs)
{
	static boost::pool_allocator<EqSyntaxNode> eq_alloc;

	return std::allocate_shared<EqSyntaxNode,
		boost::pool_allocator<EqSyntaxNode>>(eq_alloc,
			lhs, rhs);
}


SyntaxNodePtr FreeSyntaxNode::construct(size_t id)
{
	static boost::pool_allocator<FreeSyntaxNode> free_alloc;

	return std::allocate_shared<FreeSyntaxNode,
		boost::pool_allocator<FreeSyntaxNode>>(free_alloc,
			id);
}


SyntaxNodePtr ConstantSyntaxNode::construct(size_t symb_id)
{
	static boost::pool_allocator<ConstantSyntaxNode> const_alloc;

	return std::allocate_shared<ConstantSyntaxNode,
		boost::pool_allocator<ConstantSyntaxNode>>(const_alloc,
			symb_id);
}


// use the same allocator for both FuncSyntaxNode construction
// functions
static boost::pool_allocator<FuncSyntaxNode> g_func_alloc;


SyntaxNodePtr FuncSyntaxNode::construct(size_t symb_id,
	FuncSyntaxNode::Container::iterator begin,
	FuncSyntaxNode::Container::iterator end)
{
	return std::allocate_shared<FuncSyntaxNode,
		boost::pool_allocator<FuncSyntaxNode>>(g_func_alloc,
			symb_id, begin, end);
}


SyntaxNodePtr FuncSyntaxNode::move_construct(size_t symb_id,
	FuncSyntaxNode::Container::iterator begin,
	FuncSyntaxNode::Container::iterator end)
{
	return std::allocate_shared<FuncSyntaxNode,
		boost::pool_allocator<FuncSyntaxNode>>(g_func_alloc,
			symb_id, std::make_move_iterator(begin),
			std::make_move_iterator(end));
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


