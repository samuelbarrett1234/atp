#include "EquationalGrammar.h"
#include <boost/spirit/include/phoenix.hpp>


namespace atp
{
namespace logic
{
namespace grammar
{


ParseNodePtr Parser::join_siblings(
	ParseNodePtr p_head, ParseNodePtr p_tail)
{
	// head node must be an identifier node
	auto p_head_id = dynamic_cast<IdentifierParseNode*>(
		p_head.get());
	ATP_LOGIC_PRECOND(p_head_id != nullptr);

	p_head_id->set_sibling(p_tail);
	return p_head;
}


Parser::Parser() :
	Parser::base_type(start)
{
	// statements are line-separated
	start = statement % qi::eol;

	// a statement is an equality of two expressions
	statement = (expression >> '=' >> expression)
		[qi::_val = phx::construct<ParseNodePtr>(
			phx::new_<EqParseNode>(qi::_1, qi::_2)
			)];


	// an expression is either an identifier, like "x",
	// or an identifier followed by a list of expressions
	// like a function application: "f(x, y, z)"
	expression = identifier[
		// constant
		qi::_val = phx::construct<ParseNodePtr>(
			phx::new_<IdentifierParseNode>(
				qi::_1, IdentifierParseNode::ArgArray()
				)
			)
	]
		|
		// function application
		(identifier >> '(' >> expression_list >> ')')[
			qi::_val = phx::construct<ParseNodePtr>(
				phx::new_<IdentifierParseNode>(
					qi::_1, qi::_2
					)
				)
		];

	// a list of expressions is one or more comma-separated expressions
	expression_list = expression % qi::char_(',');

	// identifiers are alphanumeric strings, or arithmetic
	// operations
	identifier = +(qi::alnum | '+' | '-' | '*' | '/');
}


qi::rule<Iterator> Skipper()
{
	qi::rule<Iterator> comments = '#' >> *(qi::char_ - qi::eol) >> (qi::eol | qi ::eoi);

	return qi::space | comments;
}


}  // namespace grammar
}  // namespace logic
}  // namespace atp


