#pragma once


/*

\file

\author Samuel Barrett

\brief Contains the definitions of the syntax tree classes

\details A syntax tree represents an expression, which has been
    type-checked, and may contain user-defined constants and
	functions, and free variables. In this equational logic, an
	"expression" is what you would find on either side of an equals
	rule or a rewrite rule. This file contains information about the
	nodes in a syntax tree.

*/


#include <vector>
#include <memory>
#include "../../ATPLogicAPI.h"
#include "ParseNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ModelContext;  // forward definition


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
	EqSyntaxNode(const SyntaxNodePtr& l, const SyntaxNodePtr& r) :
		m_left(l), m_right(r)
	{
		ATP_LOGIC_PRECOND(m_left->get_type() != SyntaxNodeType::EQ);
		ATP_LOGIC_PRECOND(m_right->get_type() != SyntaxNodeType::EQ);
	}

	/**
	\brief Optimised allocation function
	*/
	static SyntaxNodePtr construct(const SyntaxNodePtr& lhs,
		const SyntaxNodePtr& rhs);

	inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::EQ;
	}
	inline const SyntaxNodePtr& left() const
	{
		return m_left;
	}
	inline const SyntaxNodePtr& right() const
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

	static SyntaxNodePtr construct(size_t id);

	inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::FREE;
	}
	inline size_t get_free_id() const
	{
		return m_id;
	}

private:
	// a 0-indexed ID for each free variable (because storing strings
	// isn't very efficient!)
	const size_t m_id;
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

	/**
	\brief Optimised allocation function
	*/
	static SyntaxNodePtr construct(size_t symb_id);

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
	const size_t m_symbol_id;
};


// function (arity >= 1, user-defined)
class ATP_LOGIC_API FuncSyntaxNode :
	public ConstantSyntaxNode
{
	// inherits from constant symbol above!
public:
	// The container for the child nodes
	typedef std::vector<SyntaxNodePtr> Container;

	/**
	\warning Please refrain from using std::make_shared for this
		class, instead use the static construct functions instead.
	*/
	template<typename IterT>
	FuncSyntaxNode(size_t symb_id,
		IterT begin,
		IterT end) :
		ConstantSyntaxNode(symb_id),
		m_children(begin, end)
	{
		// functions always need arguments
		ATP_LOGIC_PRECOND(!m_children.empty());
	}

	/**
	\brief Optimised allocation function which copies the child
		nodes in the [begin,end) range.
	*/
	static SyntaxNodePtr construct(size_t symb_id,
		Container::iterator begin,
		Container::iterator end);

	/**
	\brief Optimised allocation function which *MOVES* the child
		nodes in the [begin,end) range.
	
	\warning Only use this variant if you are sure that the
		[begin,end) range is not going to be used afterwards.
	*/
	static SyntaxNodePtr move_construct(size_t symb_id,
		Container::iterator begin,
		Container::iterator end);

	inline SyntaxNodeType get_type() const override
	{
		return SyntaxNodeType::FUNC;
	}
	inline size_t get_arity() const
	{
		return m_children.size();
	}
	inline Container::const_iterator begin() const
	{
		return m_children.begin();
	}
	inline Container::const_iterator end() const
	{
		return m_children.end();
	}
	inline Container::const_reverse_iterator rbegin() const
	{
		return m_children.rbegin();
	}
	inline Container::const_reverse_iterator rend() const
	{
		return m_children.rend();
	}

private:
	Container m_children;
};


/**

\brief Convert a parse tree into a syntax tree

\note This performs type checking, which is why we require the context

\returns nullptr if failed, otherwise returns the root to the syntax
    tree.
*/
ATP_LOGIC_API SyntaxNodePtr ptree_to_stree(ParseNodePtr ptree,
	const ModelContext& ctx);


}  // namespace equational
}  // namespace logic
}  // namespace atp


