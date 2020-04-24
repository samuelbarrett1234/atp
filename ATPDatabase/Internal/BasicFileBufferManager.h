#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a file manager which does not do any caching.

*/


#include <map>
#include <fstream>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IBufferManager.h"


namespace atp
{
namespace db
{


class ATP_DATABASE_API BasicFileBufferManager :
	public IBufferManager
{
public:
	class ATP_DATABASE_API InputStream :
		public IReadableStream
	{
	public:
		/**
		\pre `(bool)in` is true
		*/
		InputStream(std::ifstream&& in) :
			m_stream(std::move(in))
		{
			ATP_DATABASE_PRECOND((bool)m_stream);
		}

		inline std::istream& is() override
		{
			return m_stream;
		}

	private:
		std::ifstream m_stream;
	};

	class ATP_DATABASE_API InputOutputStream :
		public IReadWriteStream
	{
	public:
		/**
		\pre `(bool)fs` is true
		*/
		InputOutputStream(std::fstream&& fs) :
			m_stream(std::move(fs))
		{
			ATP_DATABASE_PRECOND((bool)m_stream);
		}

		inline std::istream& is() override
		{
			return m_stream;
		}
		inline std::ostream& os() override
		{
			return m_stream;
		}

	private:
		std::fstream m_stream;
	};

public:
	/**
	\pre All filenames here exist and are regular files with
		sufficient permissions etc.
	*/
	BasicFileBufferManager(std::map<ResourceName,
		std::string> filenames);

	std::shared_ptr<IReadableStream> request_read_access(
		ResourceName res) override;
	std::shared_ptr<IReadWriteStream> request_write_access(
		ResourceName res) override;

private:
	// mapping from resource names to file names
	const std::map<ResourceName, std::string> m_filenames;
};


}  // namespace db
}  // namespace atp


