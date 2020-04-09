#pragma once


/*

\file

\author Samuel Barrett

\brief Contains the classes that form the parse tree

\detailed This file contains the different kinds of nodes that can
    occur in an equational logic parse tree. There are only two
	kinds: equals nodes (which have an LHS and an RHS) and an
	identifier node (which can have children or siblings of other
	identifier nodes).

*/


#include <string>
#include <memory>
#include <list>
#include "../../ATPLogicAPI.h"


namespace atp
{
namespace logic
{
namespace equational
{


enum class ParseNodeType
{
	EQ, IDENTIFIER
};


/**
\interface IParseNode

\brief Base parse node interface
*/
class ATP_LOGIC_API IParseNode
{
public:
	virtual ~IParseNode() = default;

	virtual ParseNodeType get_type() const = 0;
};
typedef std::shared_ptr<IParseNode> ParseNodePtr;


/**
\brief Represents the '=' sign at the top of a statement, which has
    a LHS and a RHS.
*/
class ATP_LOGIC_API EqParseNode :
	public IParseNode
{
public:
	EqParseNode(ParseNodePtr left, ParseNodePtr right) :
		m_left(left), m_right(right)
	{
		ATP_LOGIC_PRECOND(m_left != nullptr);
		ATP_LOGIC_PRECOND(m_right != nullptr);
	}

	inline ParseNodeType get_type() const override
	{
		return ParseNodeType::EQ;
	}
	inline ParseNodePtr left() const
	{
		return m_left;
	}
	inline ParseNodePtr right() const
	{
		return m_right;
	}

private:
	// left and right of equals sign
	ParseNodePtr m_left, m_right;
};


/**
\detailed Identifiers always have a string storing their name. If an
    identifier has no children, it is just a constant, 'x'. If an
	identifier has a list of children, it is a function, and those
	child expressions are its arguments (in order).
*/
class ATP_LOGIC_API IdentifierParseNode :
	public IParseNode
{
public:
	typedef std::list<ParseNodePtr> ArgArray;

public:
	IdentifierParseNode(std::string name,
		ArgArray children) :
		m_name(name), m_children(children)
	{
		ATP_LOGIC_PRECOND(name.size() > 0);
	}
	inline ParseNodeType get_type() const override
	{
		return ParseNodeType::IDENTIFIER;
	}
	inline std::string get_name() const
	{
		return m_name;
	}
	inline ArgArray::const_iterator begin() const
	{
		return m_children.begin();
	}
	inline ArgArray::const_iterator end() const
	{
		return m_children.end();
	}
	inline ArgArray::const_reverse_iterator rbegin() const
	{
		return m_children.rbegin();
	}
	inline ArgArray::const_reverse_iterator rend() const
	{
		return m_children.rend();
	}

private:
	std::string m_name;
	ArgArray m_children;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


