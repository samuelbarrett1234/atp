#pragma once


/*

EquationalParseNodes.h

This file contains the different kinds of nodes that can occur in an
equational logic parse tree. There are only two kinds: equals nodes
(which have an LHS and an RHS) and an identifier node (which can have
children or siblings of other identifier nodes).

*/


#include <string>
#include <memory>
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


// base parse node type
class ATP_LOGIC_API IParseNode
{
public:
	virtual ~IParseNode() = default;

	virtual ParseNodeType get_type() const = 0;
};
typedef std::shared_ptr<IParseNode> ParseNodePtr;


// represents the '=' sign at the top of a statement, which has
// a LHS and a RHS.
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


// identifiers always have a string (their name).
// if an identifier has no children, it is just a constant, 'x'.
// if an identifier has a list of children, it is a function, and
// those child expressions are its arguments (in order).
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

private:
	std::string m_name;
	ArgArray m_children;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


