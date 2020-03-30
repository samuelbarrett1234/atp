#include "EquationalGrammar.h"
#include <boost/spirit/include/phoenix.hpp>
#include <boost/bind.hpp>


namespace atp
{
namespace logic
{
namespace grammar
{


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
	symbol_def_list = symbol_def % eol;

	symbol_def = Identifier() >> qi::uint_
		[qi::_val = boost::bind(&std::make_pair<std::string, size_t>,
			_1, _2)];
}


qi::rule<Iterator> Skipper()
{
	qi::rule<Iterator> comments = '#' >> *(qi::char_ - qi::eol) >> (qi::eol | qi ::eoi);

	return qi::space | comments;
}


qi::rule<Iterator, std::string, SkipperType> Identifier()
{
	return +(qi::alnum | '+' | '-' | '*' | '/' | '.' | '_' |
		'?' | '^' | '%' | '&');
}


}  // namespace grammar
}  // namespace logic
}  // namespace atp


