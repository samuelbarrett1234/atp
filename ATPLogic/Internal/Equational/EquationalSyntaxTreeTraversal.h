#pragma once


/*

EquationalSyntaxTreeTraversal.h

Contains templated helper functions for traversing syntax trees.

*/


#include "EquationalSyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


// EqFuncT : should be of type (EqSyntaxNode&) -> ResultT
// FreeFuncT : should be of type (FreeSyntaxNode&) -> ResultT
// ConstFuncT : should be of type (ConstantSyntaxNode&) -> ResultT
// FFuncT : should be of type (FuncSyntaxNode&) -> ResultT
template<typename ResultT,
	typename EqFuncT, typename FreeFuncT,
	typename ConstFuncT, typename FFuncT>
ResultT apply_to_syntax_node(EqFuncT eq_func, FreeFuncT free_func,
	ConstFuncT const_func, FFuncT f_func, ISyntaxNode& node)
{
	switch (node.get_type())
	{
	case SyntaxNodeType::EQ:
	{
		auto p_eq = dynamic_cast<EqSyntaxNode*>(&node);
		ATP_LOGIC_ASSERT(p_eq != nullptr);
		return eq_func(*p_eq);
	}
	case SyntaxNodeType::FREE:
	{
		auto p_free = dynamic_cast<FreeSyntaxNode*>(&node);
		ATP_LOGIC_ASSERT(p_free != nullptr);
		return free_func(*p_free);
	}
	case SyntaxNodeType::CONSTANT:
	{
		auto p_const = dynamic_cast<ConstantSyntaxNode*>(&node);
		ATP_LOGIC_ASSERT(p_const != nullptr);
		return const_func(*p_const);
	}
	case SyntaxNodeType::FUNC:
	{
		auto p_func = dynamic_cast<FuncSyntaxNode*>(&node);
		ATP_LOGIC_ASSERT(p_func != nullptr);
		return f_func(*p_func);
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid node type!");
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


