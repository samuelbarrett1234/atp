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
	// statements are separated by any number of lines, maybe
	// with comments in between the newlines.
	// at the very end and start, there will be potentially many
	// newlines or comments, and finally we must reach the end of
	// the input.
	start = *(skpr | qi::eol) >>
		(statement % (+(-skpr >> qi::eol)))
		>> *(skpr | qi::eol) >> qi::eoi;

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


