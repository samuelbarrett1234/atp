#pragma once


/**

\file

\author Samuel Barrett

*/


#include <string>
#include <boost/optional.hpp>
#include <ATPLogic.h>


struct ContextFile
{
	atp::logic::LangType lang_type;
	std::string definition_file_path,
		axiom_file_path;
};


boost::optional<ContextFile> parse_context_file(
	std::istream& in);


