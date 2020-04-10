/**

\file

\author Samuel Barrett

*/


#include "Grammar.h"
#include <boost/spirit/include/phoenix.hpp>
#include <boost/bind.hpp>


namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;


namespace atp
{
namespace logic
{
namespace equational
{


Skipper::Skipper() :
	Skipper::base_type(skip)
{
	comments = '#' >> *(qi::char_ - qi::eol);

	skip = comments | qi::blank;
}


StatementGrammar::StatementGrammar() :
	StatementGrammar::base_type(start)
{
	// statements are line-separated, by any number of lines
	// (always read end of input at the end, to prevent partial
	// parses - see the test `test_no_partial_load`.
	start = statement % (+qi::eol) >> *qi::eol >> qi::eoi;

	// a statement is an equality of two expressions
	statement = (expression >> '=' >> expression)
		[qi::_val = phx::construct<ParseNodePtr>(
			phx::new_<EqParseNode>(qi::_1, qi::_2)
			)];


	// an expression is either an identifier, like "x",
	// or an identifier followed by a list of expressions
	// like a function application: "f(x, y, z)"
	expression =
		// function application
		(identifier >> '(' >> expression_list >> ')')[
			qi::_val = phx::construct<ParseNodePtr>(
				phx::new_<IdentifierParseNode>(
					qi::_1, qi::_2
					)
				)
		]
		| identifier[
			// constant
			qi::_val = phx::construct<ParseNodePtr>(
				phx::new_<IdentifierParseNode>(
					qi::_1, IdentifierParseNode::ArgArray()
					)
				)
		];

	// a list of expressions is one or more comma-separated expressions
	expression_list = expression % qi::char_(',');

	identifier = Identifier();

	BOOST_SPIRIT_DEBUG_NODE(start);
	BOOST_SPIRIT_DEBUG_NODE(statement);
	BOOST_SPIRIT_DEBUG_NODE(expression);
	BOOST_SPIRIT_DEBUG_NODE(identifier);
}


DefinitionGrammar::DefinitionGrammar() :
	DefinitionGrammar::base_type(symbol_def_list)
{
	// definitions are line-separated, by any number of lines
	// (always read end of input at the end, to prevent partial
	// parses - see the test `test_no_partial_load`.
	symbol_def_list = symbol_def % (+qi::eol) >> *qi::eol >> qi::eoi;

	symbol_def = identifier >> qi::uint_;

	identifier = Identifier();

	BOOST_SPIRIT_DEBUG_NODE(symbol_def_list);
	BOOST_SPIRIT_DEBUG_NODE(symbol_def);
	BOOST_SPIRIT_DEBUG_NODE(identifier);
}


qi::rule<QiParseIterator, std::string(), Skipper> Identifier()
{
	return qi::lexeme[+(qi::alnum | qi::char_('+') | qi::char_('-')
		| qi::char_('*') | qi::char_('/') | qi::char_('.')
		| qi::char_('_') | qi::char_('?') | qi::char_('^')
		| qi::char_('%') | qi::char_('&'))];
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


