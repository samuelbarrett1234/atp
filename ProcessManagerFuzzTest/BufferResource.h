#pragma once


// Author: Samuel Barrett


#include <Resource.h>
#include <array>


using atpsearch::IResource;
using atpsearch::IBufferResource;
using atpsearch::ResourceID;


/// <summary>
/// Represents an in-memory buffer resource.
/// </summary>
class BufferResource :
	public IBufferResource
{
public:
	BufferResource(ResourceID id);

	size_t read(void* pBuf, size_t bufSize, size_t readAmount) override;

	size_t write(const void* pBuf, size_t writeAmount) override;

	void seek(std::ios_base::seekdir seekDir, std::streamoff off) override;

	inline std::string get_name() const override
	{
		return "";
	}

	inline ResourceID get_id() const override
	{
		return m_ID;
	}

	size_t read_from(IResource& res, size_t amount) override;

	size_t write_to(IResource& res, size_t amount) override;

private:
	static const size_t SIZE = 1024U;
	std::array<unsigned char, SIZE> m_Data;
	const ResourceID m_ID;
	size_t m_Offset;
};


