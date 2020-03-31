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
	qi::rule<QiParseIterator> comments = 
		'#' >> *(qi::char_ - qi::eol) >> (qi::eol | qi::eoi);

	skip = qi::space | comments;
}


StatementGrammar::StatementGrammar() :
	StatementGrammar::base_type(start)
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
	expression = Identifier()[
		// constant
		qi::_val = phx::construct<ParseNodePtr>(
			phx::new_<IdentifierParseNode>(
				qi::_1, IdentifierParseNode::ArgArray()
				)
			)
	]
		|
		// function application
		(Identifier() >> '(' >> expression_list >> ')')[
			qi::_val = phx::construct<ParseNodePtr>(
				phx::new_<IdentifierParseNode>(
					qi::_1, qi::_2
					)
				)
		];

	// a list of expressions is one or more comma-separated expressions
	expression_list = expression % qi::char_(',');
}


DefinitionGrammar::DefinitionGrammar() :
	DefinitionGrammar::base_type(symbol_def_list)
{
	symbol_def_list = symbol_def % qi::eol;

	symbol_def = Identifier() >> qi::uint_;
}


qi::rule<QiParseIterator, std::string(), Skipper> Identifier()
{
	return +(qi::alnum | '+' | '-' | '*' | '/' | '.' | '_' |
		'?' | '^' | '%' | qi::lit('&'));
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


