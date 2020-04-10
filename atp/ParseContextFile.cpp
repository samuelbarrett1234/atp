/**

\file

\author Samuel Barrett

\brief Contains context file parser and grammar

*/


#include "ParseContextFile.h"
#include <string>
#include <list>
#include <utility>  // for std::pair
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/phoenix.hpp>


namespace qi = boost::spirit::qi;
// we will be iterating over input streams
typedef boost::spirit::istream_iterator QiParseIterator;
typedef qi::blank_type SkipType;
typedef std::pair<std::string, std::string> KeyValuePair;
typedef std::list<KeyValuePair> KeyValueList;


/**
\brief A grammar for a file of "x=y" pairs
*/
struct ContextFileGrammar :
	public qi::grammar<QiParseIterator,
	KeyValueList(),
	SkipType>
{
	ContextFileGrammar() :
		ContextFileGrammar::base_type(assignment_list)
	{
		assignment_list = assignment % qi::eol;

		assignment = identifier >>
			'=' >> identifier;

		auto char_type = (qi::alnum
			| qi::char_('/')
			| qi::char_('\\')
			| qi::char_('.')
			| qi::char_('_'));

		identifier = qi::lexeme[+char_type]
			|
			'"' >> qi::lexeme[+(char_type | ' ')] >> '"';
	}

	qi::rule<QiParseIterator,
		KeyValuePair(),
		SkipType> assignment;
	qi::rule<QiParseIterator,
		KeyValueList(),
		SkipType> assignment_list;
	qi::rule<QiParseIterator,
		std::string(), SkipType> identifier;
};


boost::optional<ContextFile> parse_context_file(
	std::istream& in)
{
	in >> std::noskipws;

	QiParseIterator begin(in), end;

	KeyValueList output;
	const bool ok = qi::phrase_parse(
		begin, end, ContextFileGrammar(),
		qi::blank,
		output
	);

	if (!ok || begin != end)
	{
		return boost::none;
	}
	// else succeeded
	else
	{
		boost::optional<atp::logic::LangType> lang;
		boost::optional<std::string> def_file, ax_file;

		for (auto pair : output)
		{
			if (pair.first == "lang")
			{
				if (lang.has_value())
				{
					// redefinition
					return boost::none;
				}
				
				if (pair.second == "equational")
				{
					lang = atp::logic::LangType::EQUATIONAL_LOGIC;
				}
			}
			else if (pair.first == "definitions")
			{
				if (def_file.has_value())
				{
					// redefinition
					return boost::none;
				}

				def_file = pair.second;
			}
			else if (pair.first == "axioms")
			{
				if (ax_file.has_value())
				{
					// redefinition
					return boost::none;
				}

				ax_file = pair.second;
			}
		}

		if (lang.has_value() && def_file.has_value()
			&& ax_file.has_value())
		{
			ContextFile cf;
			cf.lang_type = *lang;
			cf.definition_file_path = *def_file;
			cf.axiom_file_path = *ax_file;
			return cf;
		}
		else
		{
			// missing data
			return boost::none;
		}
	}
}


