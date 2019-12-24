#include "BufferResource.h"
#include <Error.h>


BufferResource::BufferResource(ResourceID id) :
	m_Offset(0U),
	m_ID(id)
{
	// Flush memory
	memset(m_Data.data(), 0, SIZE);
}


size_t BufferResource::read(void* pBuf, size_t bufSize, size_t readAmount)
{
	size_t amount = std::min(bufSize, readAmount);
	amount = std::min(amount, SIZE - m_Offset);

	memcpy_s(pBuf, bufSize, m_Data.data() + m_Offset, SIZE - m_Offset);

	m_Offset += amount;

	return amount;
}


size_t BufferResource::write(const void* pBuf, size_t writeAmount)
{
	size_t amount = std::min(writeAmount, SIZE - m_Offset);

	memcpy_s(m_Data.data() + m_Offset, SIZE - m_Offset, pBuf, amount);

	m_Offset += amount;

	return amount;
}


void BufferResource::seek(std::ios_base::seekdir seekDir, std::streamoff off)
{
	ATP_CHECK_PRECOND(off <= SIZE, "Range check on streamoff.");
	
	switch (seekDir)
	{
	case std::ios_base::beg:
		m_Offset = off;
		break;
	case std::ios_base::cur:
		m_Offset += off;
		break;
	case std::ios_base::end:
		m_Offset = SIZE - off;
		break;
	}

	ATP_CHECK_POSTCOND(off <= SIZE, "Range check on buffer offset.");
}


size_t BufferResource::read_from(IResource& res, size_t amount)
{
	size_t result_amount = std::min(amount, SIZE - m_Offset);

	if (result_amount == 0U)
		return 0U;

	result_amount = res.read(m_Data.data() + m_Offset, SIZE - m_Offset, result_amount);

	m_Offset += result_amount;

	return result_amount;
}


size_t BufferResource::write_to(IResource& res, size_t amount)
{
	size_t result_amount = std::min(amount, SIZE - m_Offset);

	if (result_amount == 0U)
		return 0U;

	result_amount = res.write(m_Data.data() + m_Offset, result_amount);

	m_Offset += result_amount;

	return result_amount;
}


