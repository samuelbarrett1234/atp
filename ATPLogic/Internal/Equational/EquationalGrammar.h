#pragma once


/*

EquationalGrammar.h

This file contains the parser for the equational logic statements,
using Boost Spirit for constructing the parser from a CFG. This
can then produce a parse tree, which can then be turned into a
syntax tree.

*/


#include <string>
#include <list>
#include <boost/spirit/include/qi.hpp>
#include "EquationalParseNodes.h"


namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;


namespace atp
{
namespace logic
{
namespace equational
{


typedef boost::spirit::istream_iterator Iterator;
typedef qi::rule<Iterator> SkipperType;


// This class represents the grammar for parsing statements into
// a parse tree.
class StatementGrammar :
	public qi::grammar<Iterator, std::list<ParseNodePtr>, SkipperType>
{
public:
	StatementGrammar();

protected:
	qi::rule<Iterator, ParseNodePtr, SkipperType> statement,
		expression;
	qi::rule<Iterator, std::list<ParseNodePtr>, SkipperType> start,
		expression_list;
};


// This class represents the grammar for parsing definition files,
// which is basically just a list of (symbol name, symbol arity)
// pairs.
class DefinitionGrammar :
	public qi::grammar<Iterator,
		std::list<std::pair<std::string, size_t>>, SkipperType>
{
public:
	DefinitionGrammar();

protected:
	qi::rule<Iterator, std::pair<std::string, size_t>, SkipperType>
		symbol_def;

	qi::rule<Iterator, std::list<std::pair<std::string, size_t>>,
		SkipperType> symbol_def_list;
};


// This function returns the default skipping rule for the grammars
// above (to include skipping comments, etc).
ATP_LOGIC_API qi::rule<Iterator> Skipper();


// This is the rule for parsing an "identifier"
ATP_LOGIC_API qi::rule<Iterator, std::string, SkipperType> Identifier();


}  // namespace equational
}  // namespace logic
}  // namespace atp


