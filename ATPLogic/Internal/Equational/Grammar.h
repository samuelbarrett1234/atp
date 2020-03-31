#pragma once


/*

EquationalGrammar.h

This file contains the parser for the equational logic statements,
using Boost Spirit for constructing the parser from a CFG. This
can then produce a parse tree, which can then be turned into a
syntax tree.

*/


// #define BOOST_SPIRIT_DEBUG
#include <string>
#include <list>
#include <map>
#include <boost/spirit/include/qi.hpp>
#include "ParseNodes.h"


namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;


namespace atp
{
namespace logic
{
namespace equational
{


typedef boost::spirit::istream_iterator Iterator;


struct ATP_LOGIC_API Skipper :
	public qi::grammar<Iterator>
{
	Skipper();

	qi::rule<Iterator> skip;
};


// This class represents the grammar for parsing statements into
// a parse tree.
struct ATP_LOGIC_API StatementGrammar :
	public qi::grammar<Iterator, std::list<ParseNodePtr>()>
{
	StatementGrammar();

	qi::rule<Iterator, ParseNodePtr, Skipper> statement,
		expression;
	qi::rule<Iterator, std::list<ParseNodePtr>, Skipper> start,
		expression_list;
};


// This class represents the grammar for parsing definition files,
// which is basically just a list of (symbol name, symbol arity)
// pairs.
struct ATP_LOGIC_API DefinitionGrammar :
	public qi::grammar<Iterator,
		std::map<std::string, size_t>()>
{
	DefinitionGrammar();

	qi::rule<Iterator, std::pair<std::string, size_t>, Skipper>
		symbol_def;

	qi::rule<Iterator, std::map<std::string, size_t>,
		Skipper> symbol_def_list;
};


// This is the rule for parsing an "identifier"
ATP_LOGIC_API qi::rule<Iterator, std::string, Skipper> Identifier();


}  // namespace equational
}  // namespace logic
}  // namespace atp


