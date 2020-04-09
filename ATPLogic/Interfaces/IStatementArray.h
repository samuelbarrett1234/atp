#pragma once


/**

\file

\author Samuel Barrett

\brief Provides the IStatementArray interface for collections of
    statement objects

\details This file contains an interface to a class representing
    a one-dimensional array of statements. This interface is mostly
	useful for efficiency purposes - arrays of IStatement objects can
	be slow. Ideally uses lazy semantics where possible.

*/


#include <vector>
#include <memory>
#include "../ATPLogicAPI.h"
#include "IStatement.h"


namespace atp
{
namespace logic
{


class ATP_LOGIC_API IStatementArray;
typedef std::shared_ptr<IStatementArray> StatementArrayPtr;


/**

\interface IStatementArray

An immutable 1-dimensional array of IStatement objects.
Being an immutable collection, you cannot modify the
shape or contents once created.
It also contains strictly contiguous types (every
statement in this array is the same derived type).

*/
class ATP_LOGIC_API IStatementArray
{
public:
	virtual ~IStatementArray() = default;

	virtual size_t size() const = 0;

	/**
	\pre i < size()
	*/
	virtual const IStatement& at(size_t i) const = 0;

	/**
	\pre start <= end and end <= size() and start < size() and
	    step > 0

	\post returns the statements starting at and including the one
	    at 'start' and finishing at but NOT including the one at
	    'end', going in steps of 'step'.
	*/
	virtual StatementArrayPtr slice(size_t start, size_t end,
		size_t step = 1) const = 0;
};


}  // namespace logic
}  // namespace atp


