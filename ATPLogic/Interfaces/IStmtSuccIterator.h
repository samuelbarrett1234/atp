#pragma once


/**
\file

\author Samuel Barrett

\brief Contains an interface for an object allowing you to traverse
	a tree of successor iterators from a given start statement.

*/


#include <memory>
#include "../ATPLogicAPI.h"
#include "IStatement.h"


namespace atp
{
namespace logic
{


class IStmtSuccIterator;  // forward declaration
typedef std::shared_ptr<IStmtSuccIterator> StmtSuccIterPtr;


/**
\brief An object allowing you to traverse the successors of a given
	statement.
*/
class ATP_LOGIC_API IStmtSuccIterator
{
public:
	virtual ~IStmtSuccIterator() = default;

	/**
	\brief Advance to the next successor

	\pre valid()
	*/
	virtual void advance() = 0;

	/**
	\brief Determine if we have reached the end of the successors.

	\returns True iff we are not at the end
	*/
	virtual bool valid() const = 0;

	/**
	\brief Get the successor represented by the current iterator
		position.

	\pre valid()
	*/
	virtual const IStatement& get() const = 0;

	/**
	\brief "Dive" into the next depth layer of successors, which
		basically just means "create a successor iterator for the
		current statement".

	\returns A new successor iterator for the statement returned
		under `get()`.

	\pre valid()

	\warning The returned iterator may not be valid, if our statement
		has no successors.
	*/
	virtual StmtSuccIterPtr dive() const = 0;
};


}  // namespace logic
}  // namespace atp


