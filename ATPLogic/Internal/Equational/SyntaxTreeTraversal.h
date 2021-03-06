#pragma once


/*

\file

\author Samuel Barrett

\brief Contains a templated helper function for applying a function
    to a syntax node, without having to know its type.

*/


#include "SyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\brief Apply a different function depending on the node type

\note This is just a `map` operation from functional programming

\tparam ResultT the return result type

\tparam EqFuncT should be of type EqSyntaxNode& -> ResultT

\tparam FreeFuncT should be of type FreeSyntaxNode& -> ResultT

\tparam ConstFuncT should be of type ConstantSyntaxNode& -> ResultT

\tparam FFuncT should be of type FuncSyntaxNode& -> ResultT
*/
template<typename ResultT,
	typename EqFuncT, typename FreeFuncT,
	typename ConstFuncT, typename FFuncT>
ResultT apply_to_syntax_node(EqFuncT eq_func, FreeFuncT free_func,
	ConstFuncT const_func, FFuncT f_func, ISyntaxNode& node)
{
	// static assertions on the types of the functions given:

	static_assert(std::is_convertible<EqFuncT,
		std::function<ResultT(EqSyntaxNode&)>>::value,
		"EqFuncT should be of type (EqSyntaxNode&) -> ResultT");
	static_assert(std::is_convertible<FreeFuncT,
		std::function<ResultT(FreeSyntaxNode&)>>::value,
		"FreeFuncT should be of type (FreeSyntaxNode&) -> ResultT");
	static_assert(std::is_convertible<ConstFuncT,
		std::function<ResultT(ConstantSyntaxNode&)>>::value,
		"ConstFuncT should be of type (ConstFuncT&) -> ResultT");
	static_assert(std::is_convertible<FFuncT,
		std::function<ResultT(FuncSyntaxNode&)>>::value,
		"FFuncT should be of type (FuncSyntaxNode&) -> ResultT");

	// now proceed with the function:

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


