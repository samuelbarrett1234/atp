#include "EquationalSyntaxTrees.h"
#include <boost/bind.hpp>
#include <map>


// TODO: get rid of the recursion in this file, and use a stack
// instead.


namespace atp
{
namespace logic
{


typedef std::map<std::string, size_t> IdentifierMap;


struct ParseData
{
	IdentifierMap free_var_ids;
	size_t next_free_id;
	EquationalKnowledgeKernel& ker;
};


SyntaxNodePtr parse_any(ParseNodePtr node, ParseData& pd);
SyntaxNodePtr parse_eq(ParseNodePtr node, ParseData& pd);
SyntaxNodePtr parse_free(ParseNodePtr node, ParseData& pd);
SyntaxNodePtr parse_const(ParseNodePtr node, ParseData& pd);
SyntaxNodePtr parse_func(ParseNodePtr node, ParseData& pd);


SyntaxNodePtr ptree_to_stree(ParseNodePtr ptree)
{
	IdentifierMap free_var_ids;
	size_t next_free_id = 0;

	return parse_any(ptree, free_var_ids, next_free_id);
}


SyntaxNodePtr parse_any(ParseNodePtr ptree, ParseData& pd)
{
	switch (ptree->get_type())
	{
	case ParseNodeType::EQ:
		return parse_eq(ptree, pd);
	case ParseNodeType::IDENTIFIER:
	{
		const auto& idn = dynamic_cast<const IdentifierParseNode&>(
			*ptree);

		// we need to be careful about which of the following this
		// identifier represents: a free variable, a user-defined
		// constant, or a user-defined function application.
		
		if (idn.begin() != idn.end())
		{
			// if it has children then it is definitely a function
			// call:
			return parse_func(ptree, pd);
		}
		else if (pd.ker.is_defined(idn.get_name()))
		{
			// it has no children but is user-defined, thus it is a
			// user-defined constant:
			return parse_const(ptree, pd);
		}
		// otherwise it must be free:
		else return parse_free(ptree, pd);
	}
	default:
		return nullptr;
	}
}


SyntaxNodePtr parse_eq(ParseNodePtr node, ParseData& pd)
{
	ATP_LOGIC_PRECOND(node->get_type() == ParseNodeType::EQ);

	const auto& eq = dynamic_cast<const EqParseNode&>(*node);

	auto p_left = parse_any(eq.get_left(), pd);
	auto p_right = parse_any(eq.get_right(), pd);

	if (p_left == nullptr || p_right == nullptr)
		return nullptr;

	return std::make_shared<EqSyntaxNode>(p_left, p_right);
}


SyntaxNodePtr parse_free(ParseNodePtr node, ParseData& pd)
{
	ATP_LOGIC_PRECOND(node->get_type() == ParseNodeType::IDENTIFIER);

	const auto& idn = dynamic_cast<const IdentifierParseNode&>(*node);

	// check that the node has no children (otherwise it would be a
	// function call, not a free variable.)
	ATP_LOGIC_PRECOND(idn.begin() == idn.end());

	// check that the identifier is not a user defined symbol
	// (otherwise it would not be free).
	ATP_LOGIC_PRECOND(!pd.ker.is_defined(idn.get_name()));

	auto name_iter = pd.free_var_ids.find(idn.get_name());

	if (name_iter != pd.free_var_ids.end())
	{
		// use existing definition of this free variable's name
		return std::make_shared<FreeSyntaxNode>(name_iter->second);
	}
	else
	{
		// create a new association between this identifier name and
		// this free variable ID
		pd.free_var_ids[idn.get_name()] = pd.next_free_id;
		pd.next_free_id++;

		return std::make_shared<FreeSyntaxNode>(pd.next_free_id - 1);
	}
}


SyntaxNodePtr parse_const(ParseNodePtr node, ParseData& pd)
{
	ATP_LOGIC_PRECOND(node->get_type() == ParseNodeType::IDENTIFIER);

	const auto& idn = dynamic_cast<const IdentifierParseNode&>(*node);

	// check that the node has no children (otherwise it would be a
	// function call, not a free variable.)
	ATP_LOGIC_PRECOND(idn.begin() == idn.end());

	// check that the identifier is a user defined symbol
	ATP_LOGIC_PRECOND(pd.ker.is_defined(idn.get_name()));

	// now see if the user is using the correct arity:
	if (pd.ker.symbol_arity(idn.get_name()) != 0)
		return nullptr;

	// compute the kernel's special assigned ID for this symbol
	const auto symb_id = pd.ker.symbol_id(idn.get_name());

	return std::make_shared<ConstantSyntaxNode>(symb_id);
}


SyntaxNodePtr parse_func(ParseNodePtr node, ParseData& pd)
{
	ATP_LOGIC_PRECOND(node->get_type() == ParseNodeType::IDENTIFIER);

	const auto& idn = dynamic_cast<const IdentifierParseNode&>(*node);

	// check that the node has children
	ATP_LOGIC_PRECOND(idn.begin() != idn.end());

	// check that the identifier is a user defined symbol
	ATP_LOGIC_PRECOND(pd.ker.is_defined(idn.get_name()));

	// the arity which has been implied by the user's usage of the
	// function:
	const size_t implied_arity = std::distance(idn.begin(),
		idn.end());

	// now see if the user is using the correct arity:
	if (pd.ker.symbol_arity(idn.get_name()) != implied_arity)
		return nullptr;

	// compute the kernel's special assigned ID for this symbol
	const auto symb_id = pd.ker.symbol_id(idn.get_name());

	// convert children over:
	std::list<SyntaxNodePtr> children;
	std::transform(idn.begin(), idn.end(),
		std::back_inserter(children),
		boost::bind(&parse_any, _1, boost::ref(pd)));

	// if any child fails then we fail too:
	if (std::any_of(children.begin(), children.end(),
		[](SyntaxNodePtr p) { return p == nullptr; }))
		return nullptr;

	// else return success
	return std::make_shared<FuncSyntaxNode>(symb_id, children);
}


}  // namespace logic
}  // namespace atp


