#pragma once


/*

EquationalSyntaxTrees.h

A syntax tree represents an expression, which has been type-checked,
and may contain user-defined constants and functions, and free
variables. In this equational logic, an "expression" is what you
would find on either side of an equals rule or a rewrite rule.

This file contains information about the nodes in a syntax tree, and
a data structure for storing and querying many trees at once.

*/


#include <list>
#include <memory>
#include <vector>
#include <functional>
#include <boost/optional.hpp>
#include "../../ATPLogicAPI.h"
#include "EquationalKnowledgeKernel.h"
#include "EquationalParseNodes.h"


namespace atp
{
namespace logic
{


// The different kinds of node we can have in our syntax tree
enum class SyntaxNodeType
{
	EQ, FREE, CONSTANT, FUNC
};


class ATP_LOGIC_API ISyntaxNode
{
public:
	virtual ~ISyntaxNode() = default;

	virtual SyntaxNodeType get_type() const = 0;
};
typedef std::shared_ptr<ISyntaxNode> SyntaxNodePtr;


// An equals symbol with one expression to either side
class ATP_LOGIC_API EqSyntaxNode :
	public ISyntaxNode
{
public:
	EqSyntaxNode(SyntaxNodePtr l, SyntaxNodePtr r) :
		m_left(l), m_right(r)
	{
		ATP_LOGIC_PRECOND(m_left->get_type() != SyntaxNodeType::EQ);
		ATP_LOGIC_PRECOND(m_right->get_type() != SyntaxNodeType::EQ);
	}
	inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::EQ;
	}
	inline SyntaxNodePtr left() const
	{
		return m_left;
	}
	inline SyntaxNodePtr right() const
	{
		return m_right;
	}
private:
	SyntaxNodePtr m_left, m_right;
};


// free variable (always arity 0)
class ATP_LOGIC_API FreeSyntaxNode :
	public ISyntaxNode
{
public:
	FreeSyntaxNode(size_t id) :
		m_id(id)
	{ }

	inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::FREE;
	}
	inline size_t get_free_id() const
	{
		return m_id;
	}

	// sometimes one needs to "rebuild" free variable IDs, after
	// substitutions etc. this function helps do that.
	inline void rebuild_free_id(size_t id)
	{
		m_id = id;
	}
private:
	// a 0-indexed ID for each free variable (because storing strings
	// isn't very efficient!)
	size_t m_id;
};


// constant (always arity 0, and user-defined)
class ATP_LOGIC_API ConstantSyntaxNode :
	public ISyntaxNode
{
	// functions inherit from this!
public:
	ConstantSyntaxNode(size_t symb_id) :
		m_symbol_id(symb_id)
	{ }

	// need virtual so that FuncSyntaxNode can override
	virtual inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::CONSTANT;
	}
	inline size_t get_symbol_id() const
	{
		return m_symbol_id;
	}
protected:
	// an ID for the symbol given to us by the kernel (because
	// actually storing the symbol names isn't very efficient!)
	// [not necessarily zero-indexed].
	size_t m_symbol_id;
};


// function (arity >= 1, user-defined)
class ATP_LOGIC_API FuncSyntaxNode :
	public ConstantSyntaxNode
{
	// inherits from constant symbol above!
public:
	FuncSyntaxNode(size_t symb_id,
		std::list<SyntaxNodePtr> arg_children) :
		ConstantSyntaxNode(symb_id),
		m_children(arg_children)
	{
		// functions always need arguments
		ATP_LOGIC_PRECOND(!m_children.empty());
	}
	inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::FUNC;
	}
	inline size_t get_arity() const
	{
		return m_children.size();
	}
	inline std::list<SyntaxNodePtr>::const_iterator begin() const
	{
		return m_children.begin();
	}
	inline std::list<SyntaxNodePtr>::const_iterator end() const
	{
		return m_children.end();
	}
private:
	std::list<SyntaxNodePtr> m_children;
};


// convert a parse_statements tree to a syntax tree
// returns nullptr iff type checking failed
ATP_LOGIC_API SyntaxNodePtr ptree_to_stree(ParseNodePtr ptree);


}  // namespace logic
}  // namespace atp


