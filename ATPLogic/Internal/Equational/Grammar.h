#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the Boost Spirit and Boost Qi grammar specifications
    for the equational logic parsing.

\details This file contains the parser for the equational logic
statements, using Boost Spirit for constructing the parser from a
CFG. This can then produce a parse tree, which can then be turned
into a syntax tree.

\note Boost's Qi library doesn't seem to let us export the
    parsers, so they aren't being exported here.

\note Using this prints out parsing info to std::cout to help
	debug parser errors: #define BOOST_SPIRIT_DEBUG

*/


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


// we will be iterating over input streams
typedef boost::spirit::istream_iterator QiParseIterator;


/**
\brief This is the skip parser, which tells the parser what to ignore
    (what is not code).

\details We skip whitespace (of course) but this also includes
    comments.
*/
struct Skipper :
	public boost::spirit::qi::grammar<QiParseIterator>
{
	Skipper();

	boost::spirit::qi::rule<QiParseIterator> skip, comments;
};


/**
\brief Grammar for statement objects (producing parse trees)

\details This is the grammar for an array of statements (which
    involve equality signs, i.e. "A=B" is a statement). It turns them
    into a "parse tree".
*/
struct StatementGrammar :
	public boost::spirit::qi::grammar<QiParseIterator,
	std::list<ParseNodePtr>(),
		Skipper>
{
	StatementGrammar();

	boost::spirit::qi::rule<QiParseIterator, ParseNodePtr(), Skipper>
		statement, expression;
	boost::spirit::qi::rule<QiParseIterator, std::list<ParseNodePtr>(),
		Skipper> start, expression_list;
	boost::spirit::qi::rule<QiParseIterator, std::string(),
		Skipper> identifier;
	Skipper skpr;
};


/**
\brief Grammar for definition files (producing name-arity pairs.)

\details This class represents the grammar for parsing definition
    files, which is basically just a list of (symbol name, symbol
	arity) pairs.
*/
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

	boost::spirit::qi::rule<QiParseIterator, std::string(),
		Skipper> identifier;

	Skipper skpr;
};


/**
\brief Parse rule for identifiers

\details An identifier is a kind of name, for a free variable,
    constant or function. Includes alphanumeric and some symbols.
*/
boost::spirit::qi::rule<QiParseIterator, std::string(),
	Skipper> Identifier();


}  // namespace equational
}  // namespace logic
}  // namespace atp


