/**
\file

\author Samuel Barrett

*/


#include "ATPDatabase.h"
#include "Internal/EquationalDatabase.h"


namespace atp
{
namespace db
{


DatabasePtr load_from_file(
	const std::string& filename,
	logic::LangType lang_type)
{
	switch (lang_type)
	{
	case logic::LangType::EQUATIONAL_LOGIC:
		return EquationalDatabase::load_from_file(filename);
	default:
		return nullptr;  // bad type
	}
}


}  // namespace db
}  // namespace atp


