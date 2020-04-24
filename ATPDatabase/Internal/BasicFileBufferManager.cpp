/**
\file

\author Samuel Barrett
*/


#include <boost/filesystem.hpp>
#include "BasicFileBufferManager.h"


namespace atp
{
namespace db
{


BasicFileBufferManager::BasicFileBufferManager(
	std::map<ResourceName, std::string> filenames) :
	m_filenames(std::move(filenames))
{
	// precondition: all filenames here are valid and they exist
	ATP_DATABASE_PRECOND(std::all_of(m_filenames.begin(),
		m_filenames.end(), [](const auto& pair)
		{
			return boost::filesystem::is_regular_file(pair.second);
		}));
}


std::shared_ptr<IReadableStream>
BasicFileBufferManager::request_read_access(ResourceName res)
{
	auto iter = m_filenames.find(res);  // thread safe because const

	ATP_DATABASE_PRECOND(iter != m_filenames.end());

	std::ifstream in(iter->second);

	if ((bool)in)
	{
		return std::make_shared<InputStream>(std::move(in));
	}
	else
	{
		return nullptr;
	}
}


std::shared_ptr<IReadWriteStream>
BasicFileBufferManager::request_write_access(ResourceName res)
{
	auto iter = m_filenames.find(res);  // thread safe because const

	ATP_DATABASE_PRECOND(iter != m_filenames.end());

	std::fstream fs(iter->second);

	if ((bool)fs)
	{
		return std::make_shared<InputOutputStream>(std::move(fs));
	}
	else
	{
		return nullptr;
	}
}


}  // namespace db
}  // namespace atp
