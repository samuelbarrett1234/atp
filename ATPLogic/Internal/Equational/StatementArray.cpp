#include "StatementArray.h"
#include <algorithm>
#include <initializer_list>


namespace atp
{
namespace logic
{
namespace equational
{


StatementArrayPtr StatementArray::try_from_stmt(
	const IStatement& _stmt)
{
	if (auto p_stmt =
		dynamic_cast<const Statement*>(&_stmt))
	{
		// create an array and then add the statement
		ArrPtr p_arr = std::make_shared<ArrType>();
		p_arr->push_back(*p_stmt);

		// now embed into one of our array objects
		auto p_result = std::make_shared<StatementArray>(
			p_arr
			);

		ATP_LOGIC_ASSERT(p_result->size() == 1);

		return p_result;
	}
	// else not a type we are concerned with
	else return StatementArrayPtr();
}


StatementArrayPtr StatementArray::try_concat(
	const IStatementArray& _l, const IStatementArray& _r)
{
	if (auto p_l = dynamic_cast<const StatementArray*>(&_l))
	{
		if (auto p_r = dynamic_cast<const StatementArray*>(&_r))
		{
			// Don't do a lazy operation; explicitly construct the
			// array. Also, another warning: both l and r might be
			// slices, with step > 1!
			// Unfortunately, this might mean a lot of copying of
			// statement objects...

			ArrType result;
			result.reserve(p_l->size() + p_r->size());

			for (size_t i = 0; i < p_l->size(); i++)
			{
				// the reinterpret_cast is safe because we already
				// know that p_l is of the StatementArray
				// type!
				result.push_back(
					reinterpret_cast<const Statement&>(
						p_l->at(i)));
			}
			for (size_t i = 0; i < p_r->size(); i++)
			{
				// the reinterpret_cast is safe because we already
				// know that p_l is of the StatementArray
				// type!
				result.push_back(
					reinterpret_cast<const Statement&>(
						p_r->at(i)));
			}

			return std::make_shared<StatementArray>(
				std::make_shared<ArrType>(std::move(result)),
				0, result.size(), 1
				);
		}
	}

	// if we get to here then we can't handle the concat operation:
	return StatementArrayPtr();
}


StatementArrayPtr StatementArray::try_concat(
	const std::vector<StatementArrayPtr>& stmts)
{
	size_t total_size = 0;

	for (const auto& stmt_arr : stmts)
	{
		if (!dynamic_cast<const StatementArray*>(
			stmt_arr.get()))
			return StatementArrayPtr();  // can't handle the type!
		
		// compute total result size while we're at it
		total_size += stmt_arr->size();
	}

	// Don't do a lazy operation; explicitly construct the
	// array. Also, another warning: arrays might be
	// slices, with step > 1!
	// Unfortunately, this might mean a lot of copying of
	// statement objects...

	ArrType result;
	result.reserve(total_size);

	for (const auto& _stmt_arr : stmts)
	{
		// the reinterpret_cast is safe because we already
		// know that stmt_arr is of the StatementArray
		// type!
		const auto& stmt_arr = reinterpret_cast<
			const StatementArray&>(_stmt_arr);

		for (size_t i = 0; i < stmt_arr.size(); i++)
		{
			// the reinterpret_cast is safe because we already
			// know that stmt_arr is of the StatementArray
			// type!
			result.push_back(
				reinterpret_cast<const Statement&>(
					stmt_arr.at(i)));
		}
	}

	return std::make_shared<StatementArray>(
		std::make_shared<ArrType>(std::move(result)),
		0, result.size(), 1
		);
}


StatementArray::StatementArray(ArrPtr p_array,
	size_t start, size_t end, size_t step) :
	m_array(p_array), m_start(start),
	m_stop(std::min(end,
		(p_array == nullptr) ? 0 : p_array->size()
		)),
	m_step(step)
{
	ATP_LOGIC_PRECOND(m_start <= m_stop);
	ATP_LOGIC_PRECOND(m_step > 0);

	if (m_array == nullptr)
	{
		m_array = std::make_shared<ArrType>();
	}

	// m_array should be non-null and we should have rectified m_stop
	// earlier in the constructor
	ATP_LOGIC_ASSERT(m_array != nullptr);
	ATP_LOGIC_ASSERT(m_stop <= m_array->size());
}


StatementArrayPtr StatementArray::slice(size_t start,
	size_t end, size_t step) const
{
	// WARNING: don't forget that the arguments given are indices
	// with respect to THIS OBJECTS's indices, not necessarily
	// m_array's!

	ATP_LOGIC_PRECOND(start <= end);
	ATP_LOGIC_PRECOND(step > 0);
	ATP_LOGIC_PRECOND(end <= size());

#ifdef ATP_LOGIC_DEFENSIVE
	// in debug mode we want to ensure that the transformation we
	// make below does not alter the expected size of the array
	// returned to the user.
	const size_t expected_size = compute_slice_size(
		start, end, step
	);
#endif

	// transform to our index space:
	start = m_start + start * m_step;
	end = m_start + end * m_step;
	step *= m_step;

	// compute result:
	auto p_result = std::make_shared<StatementArray>(
			m_array, start, end, step
		);

#ifdef ATP_LOGIC_DEFENSIVE
	// assert the size was correctly computed:
	ATP_LOGIC_ASSERT(p_result->size() == expected_size);
#endif

	return p_result;
}


size_t compute_slice_size(size_t start, size_t end, size_t step)
{
	ATP_LOGIC_PRECOND(start <= end);
	ATP_LOGIC_PRECOND(step > 0);

	return (end - start) / step +
		((end != start && (end - start) % step != 0) ? 1 : 0);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


