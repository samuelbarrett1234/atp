#pragma once


#include "Utility.h"
#include <streambuf>
#include <memory>


// Author: Samuel Barrett


namespace atpsearch
{


/// <summary>
/// A unique identifier associated with a resource.
/// Examples of resources: files/sockets/pipes/buffers.
/// </summary>
typedef size_t ResourceID;


/// <summary>
/// An interface for dealing with resources (which are streams that can
/// be read/written from/to).
/// Note that not all resources support all operations. In cases where
/// invalid operations are performed, make an assertion fail (as it is
/// the responsibility of the process implementer to ensure resources
/// are accessed correctly.)
/// </summary>
class ATP_API IResource
{
public:
	virtual ~IResource() = default;


	/// <summary>
	/// Read from this resource into a given buffer. Note that this function
	/// blocks until the operation is complete.
	/// </summary>
	/// <param name="pBuf">A pointer to the buffer to read into.</param>
	/// <param name="bufSize">The size (in bytes) of the buffer being read into.</param>
	/// <param name="readAmount">The number of bytes to read.</param>
	/// <returns>The number of bytes actually read (min of the bufSize, readAmount, and number of bytes left in the stream).</returns>
	virtual size_t read(void* pBuf, size_t bufSize, size_t readAmount) = 0;


	/// <summary>
	/// Write to this resource from a given buffer. Note that this function
	/// blocks until the operation is complete.
	/// </summary>
	/// <param name="pBuf">A pointer to the buffer to written from.</param>
	/// <param name="writeAmount">The number of bytes to write.</param>
	/// <returns>The number of bytes actually written (min of the writeAmount and capacity of the stream).</returns>
	virtual size_t write(const void* pBuf, size_t writeAmount) = 0;


	/// <summary>
	/// Change position of the read and write heads.
	/// </summary>
	/// <param name="seekDir">The direction to seek in (beg/cur/end)</param>
	/// <param name="off">The seek offset.</param>
	virtual void seek(std::ios_base::seekdir seekDir, std::streamoff off) = 0;


	/// <summary>
	/// Get a human-readable name of this resource.
	/// </summary>
	/// <returns>The name of this resource (ideally unique).</returns>
	virtual std::string get_name() const = 0;


	/// <summary>
	/// Get the unique ID of this resource.
	/// These should be set by the user of this library
	/// i.e. they aren't generated automatically.
	/// </summary>
	/// <returns>The unique resource ID.</returns>
	virtual ResourceID get_id() const = 0;
};


class ATP_API IBufferResource :
	public IResource
{
public:
	virtual ~IBufferResource() = default;


	/// <summary>
	/// Read an amount from the given resource directly
	/// into this resource (without going via a buffer).
	/// </summary>
	/// <param name="res">The resource to read from.</param>
	/// <param name="amount">The number of bytes to read.</param>
	/// <returns>The number of bytes actually read (min of amount and space left in buffer).</returns>
	virtual size_t read_from(IResource& res, size_t amount) = 0;


	/// <summary>
	/// Write an amount from this resource directly
	/// into the given resource (without going via a buffer).
	/// </summary>
	/// <param name="res">The rsource to write to.</param>
	/// <param name="amount">The number of bytes to write.</param>
	/// <returns>The number of bytes actually written (min of amount, content left in this buffer, and size of other buffer).</returns>
	virtual size_t write_to(IResource& res, size_t amount) = 0;
};


typedef std::shared_ptr<IResource> ResourcePtr;


} // namespace atpsearch


