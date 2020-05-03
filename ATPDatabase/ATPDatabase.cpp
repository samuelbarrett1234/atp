/**
\file

\author Samuel Barrett

*/


#include "ATPDatabase.h"
#include "Internal/SQLiteDatabase.h"


namespace atp
{
namespace db
{


DatabasePtr load_from_file(
	const std::string& filename,
	logic::LangType lang_type)
{
	// ignore language type (for now)
	return SQLiteDatabase::load_from_file(filename);
}


}  // namespace db
}  // namespace atp


