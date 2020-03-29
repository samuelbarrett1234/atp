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
private:
	// combine a head identifier node with a tail node as siblings
	static ParseNodePtr join_siblings(ParseNodePtr p_head,
		ParseNodePtr p_tail)
	{
		// head node must be an identifier node
		auto p_head_id = dynamic_cast<IdentifierParseNode*>(
			p_head.get());
		ATP_LOGIC_PRECOND(p_head_id != nullptr);

		p_head_id->set_sibling(p_tail);
		return p_head;
	}

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


