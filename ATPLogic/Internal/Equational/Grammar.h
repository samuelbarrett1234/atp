#pragma once


/*

EquationalGrammar.h

This file contains the parser for the equational logic statements,
using Boost Spirit for constructing the parser from a CFG. This
can then produce a parse tree, which can then be turned into a
syntax tree.

Note that Boost's Qi library doesn't seem to let us export the
parsers, so they aren't being exported here.

*/


// #define BOOST_SPIRIT_DEBUG
#include <string>
#include <list>
#include <utility>  // for std::pair
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include "ParseNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


typedef boost::spirit::istream_iterator QiParseIterator;


struct Skipper :
	public boost::spirit::qi::grammar<QiParseIterator>
{
	Skipper();

	boost::spirit::qi::rule<QiParseIterator> skip;
};


// This class represents the grammar for parsing statements into
// a parse tree.
struct StatementGrammar :
	public boost::spirit::qi::grammar<QiParseIterator, std::list<ParseNodePtr>(),
		Skipper>
{
	StatementGrammar();

	boost::spirit::qi::rule<QiParseIterator, ParseNodePtr(), Skipper>
		statement, expression;
	boost::spirit::qi::rule<QiParseIterator, std::list<ParseNodePtr>(),
		Skipper> start, expression_list;
};


// This class represents the grammar for parsing definition files,
// which is basically just a list of (symbol name, symbol arity)
// pairs.
struct DefinitionGrammar :
	public boost::spirit::qi::grammar<QiParseIterator,
		std::list<std::pair<std::string, size_t>>(),
		Skipper>
{
	DefinitionGrammar();

	boost::spirit::qi::rule<QiParseIterator,
		std::list<std::pair<std::string, size_t>>(),
		Skipper> symbol_def_list;

	boost::spirit::qi::rule<QiParseIterator,
		std::pair<std::string, size_t>(), Skipper> symbol_def;
};


// This is the rule for parsing an "identifier"
boost::spirit::qi::rule<QiParseIterator, std::string(),
	Skipper> Identifier();


}  // namespace equational
}  // namespace logic
}  // namespace atp


