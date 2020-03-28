#pragma once


/*

IStatementArray.h

This file contains an interface to a class representing an N-dimensional array of statements.
This class is necessary for efficiency purposes only - arrays of IStatement objects are too slow.
It can also implement lazy semantics where possible.

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


/// <summary>
/// A 1-dimensional array of IStatement objects.
/// This is an immutable collection (you cannot modify
/// the shape or contents once created).
/// It also contains strictly contiguous types (every
/// statement in this array is the same derived type).
/// </summary>
class ATP_LOGIC_API IStatementArray
{
public:
	virtual ~IStatementArray() = default;

	virtual size_t size() const = 0;

	// Precondition: i < size()
	virtual const IStatement& at(size_t i) const = 0;

	// Precondition: start <= end, end <= size(),
	// start < size(), step > 0.
	// Postcondition: returns the statements starting at and including
	// the one at 'start' and finishing at but NOT including the one at
	// 'end', going in steps of 'step'.
	virtual StatementArrayPtr slice(size_t start, size_t end,
		size_t step = 1) const = 0;
};


// Postcondition: returns a singleton array which contains just this
// one statement.
ATP_LOGIC_API StatementArrayPtr from_statement(const IStatement& stmt);


// Postcondition: returns the concatenation of the two arrays
// with 'l' placed first
ATP_LOGIC_API StatementArrayPtr concat(const IStatementArray& l,
	const IStatementArray& r);


// Postcondition: returns the concatenation of the array of arrays
// returning a single 1-dimensional array.
ATP_LOGIC_API StatementArrayPtr concat(
	const std::vector<StatementArrayPtr>& stmts);


}  // namespace logic
}  // namespace atp


