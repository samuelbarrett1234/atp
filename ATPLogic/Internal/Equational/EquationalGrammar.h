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
namespace grammar
{


typedef boost::spirit::istream_iterator Iterator;
typedef qi::rule<Iterator> SkipperType;


class Parser :
	public qi::grammar<Iterator, std::list<ParseNodePtr>, SkipperType>
{
public:
	using Iterator = std::string::const_iterator;

public:
	Parser();

protected:
	qi::rule<Iterator, ParseNodePtr, SkipperType> statement,
		expression;
	qi::rule<Iterator, std::string, SkipperType> identifier;
	qi::rule<Iterator, std::list<ParseNodePtr>, SkipperType> start,
		expression_list;
};


ATP_LOGIC_API qi::rule<Iterator> Skipper();


}  // namespace grammar
}  // namespace logic
}  // namespace atp


