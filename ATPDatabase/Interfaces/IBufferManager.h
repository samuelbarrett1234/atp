#pragma once


/**
\file

\author Samuel Barrett

\brief Contains interfaces relating to memory buffers.
*/


#include <memory>
#include <istream>
#include <ostream>
#include "../ATPDatabaseAPI.h"
#include "Data.h"


namespace atp
{
namespace db
{


class IReadableStream;  // forward declaration
class IReadWriteStream;  // forward declaration


/**
\brief This class abstracts away the construction of these data
	streams.

\details Buffer managers (i) translate from resource names to file
	names where necessary, (ii) can connect across networks if
	they are designed to work that way, (iii) can also implement
	buffering to keep some file contents in memory to prevent loading
	data many times.

\note It is crucial that buffer managers ensure that data is saved
	when a writable stream is written to, otherwise updates could get
	lost if the program crashes.

\note This class will be thread safe.
*/
class ATP_DATABASE_API IBufferManager
{
public:
	virtual ~IBufferManager() = default;

	/**
	\brief Get a read-only stream for the given resource.

	\note This mimicks the lock manager's corresponding function. You
		may wish to use the lock manager to prevent several accesses.

	\note It is up to the implementation if you are allowed to have
		many readers of a given file. If there is already a reader
		and this cannot be done, nullptr is returned.

	\note The returned stream is used to reference-count the number
		of owners of a particular resource. Do not hold onto it for
		longer than you need to.

	\pre `res` is a valid resource name.

	\returns Nullptr on failure, otherwise returns a new object.
	*/
	virtual std::shared_ptr<IReadableStream> request_read_access(
		ResourceName res) = 0;

	/**
	\brief Get a read-write stream for the given resource.

	\note This mimicks the lock manager's corresponding function. You
		may wish to use the lock manager to prevent several accesses.

	\note The returned stream is used to reference-count the number
		of owners of a particular resource. Do not hold onto it for
		longer than you need to.

	\pre `res` is a valid resource name.

	\returns Nullptr on failure, otherwise returns a new object. Note
		that nullptr is returned if there is already an object
		accessing this object.
	*/
	virtual std::shared_ptr<IReadWriteStream> request_write_access(
		ResourceName res) = 0;
};


/**
\brief Represents a readable buffer.

\note This object will typically be used for reference-counting the
	resources, so don't hold onto it for longer than necessary.
*/
class ATP_DATABASE_API IReadableStream
{
public:
	virtual ~IReadableStream() = default;

	/**
	\brief Return a reference to the corresponding input stream.
	*/
	virtual std::istream& is() = 0;
};


/**
\brief Represents a read/write buffer.

\note This object will typically be used for reference-counting the
	resources, so don't hold onto it for longer than necessary.
*/
class ATP_DATABASE_API IReadWriteStream :
	public virtual IReadableStream
{
public:
	virtual ~IReadWriteStream() = default;

	/**
	\brief Return a reference to the corresponding output stream.
	*/
	virtual std::ostream& os() = 0;
};


}  // namespace db
}  // namespace atp


