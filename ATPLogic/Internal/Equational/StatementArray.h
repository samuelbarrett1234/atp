#pragma once


/*

EquationalStatement.h

Implementation of the IStatementArray interface for equational logic.
This class tries to be as lazy as possible, for example by sharing a
pointer to the original array. Note that this has the downside of
potentially keeping around more memory than necessary, but I think
this is worth the speedup.

*/


#include <memory>
#include <vector>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatementArray.h"
#include "Statement.h"


namespace atp
{
namespace logic
{
namespace equational
{


// Helper function for computing the number of elements in a slice.
// Precondition: start <= end and step > 0
// Postcondition: returns the size of the set
// { k >= 0 s.t. start + k * step < end }
ATP_LOGIC_API size_t compute_slice_size(size_t start, size_t end,
	size_t step);


class ATP_LOGIC_API EquationalStatementArray : public IStatementArray
{
public:
	// We DEFINITELY want to be using a vector of the derived type
	// EquationalStatement here, not a pointer to the base type
	// IStatement.
	typedef std::vector<EquationalStatement> ArrType;
	typedef std::shared_ptr<ArrType> ArrPtr;

public:
	// these functions return nullptr if the statement types given as
	// argument are not equational (thus this class isn't responsible
	// for handling them). Returning nullptr is NOT an error.
	static StatementArrayPtr try_from_stmt(const IStatement& stmt);
	static StatementArrayPtr try_concat(const IStatementArray& l,
		const IStatementArray& r);
	static StatementArrayPtr try_concat(
		const std::vector<StatementArrayPtr>& stmts);

public:
	// start/end/step are like the parameters given to slice() and
	// they follow the same preconditions.
	EquationalStatementArray(ArrPtr p_array = ArrPtr(),
		size_t start = 0, size_t end = static_cast<size_t>(-1),
		size_t step = 1);

	inline size_t size() const override
	{
		return compute_slice_size(m_start, m_stop, m_step);
	}
	const IStatement& at(size_t i) const override
	{
		ATP_LOGIC_PRECOND(i < size());
		ATP_LOGIC_ASSERT(m_array != nullptr);
		return static_cast<const IStatement&>(
			m_array->at(m_start + i * m_step));
	}
	StatementArrayPtr slice(size_t start, size_t end,
		size_t step = 1) const override;

	// raw array can be useful for efficiency of other equational
	// systems
	inline const ArrType& raw() const
	{
		return *m_array;
	}

private:
	ArrPtr m_array;

	// invariant: m_start >= 0, m_stop <= m_array->size(),
	// m_step >= 1
	const size_t m_start, m_stop, m_step;

	// invariant: the elements represented by this array are given by
	// the indices m_start + k * m_step for k >= 0 whenever that
	// index is < m_stop.
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


