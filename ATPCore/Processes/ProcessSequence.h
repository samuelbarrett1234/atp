#pragma once


/**
\file

\author Samuel Barrett

\brief Contains templated classes to make it easier to create
	lists of processes which run in sequence.

*/


#include <type_traits>
#include <functional>
#include "../ATPCoreAPI.h"
#include "IProcess.h"


namespace atp
{
namespace core
{


/**
\brief This process just runs two processes in sequence, allowing
	them to communicate some data via the DataT object.

\tparam DataT A blob of data which is shared between the two
	processes in their creators. DataT must be default-constructible.

\tparam Proc1Creator Create the first process. Must be of type
	DataT& -> ProcessPtr. This function is not kept beyond the
	constructor.

\tparam Proc2Creator Create the second process. Must be of type
	DataT& -> ProcessPtr. This function **is** kept beyond the
	constructor, and is invoked only when the first process
	finishes.
*/
template<typename DataT, typename Proc1Creator,
	typename Proc2Creator>
class ProcessSequence :
	public IProcess
{
public:
	ProcessSequence(Proc1Creator p1, Proc2Creator p2) :
		m_create_proc2(std::move(p2)),
		m_idx(0), m_failed(false)
	{
		static_assert(std::is_convertible_v<Proc1Creator,
			ProcessPtr(DataT&)>, "Proc1Creator must be "
			"of type DataT& -> ProcessPtr");

		m_current = p1(m_data);
		ATP_CORE_PRECOND(m_current != nullptr);
	}

	inline bool done() const override
	{
		return m_idx == 2;
	}

	inline bool waiting() const override
	{
		ATP_CORE_PRECOND(!done());
		ATP_CORE_ASSERT(m_current != nullptr);
		return m_current->waiting();
	}

	inline bool has_failed() const override
	{
		return m_failed;
	}

	void run_step() override
	{
		ATP_CORE_ASSERT(m_current != nullptr);

		if (m_current->done() && !m_current->has_failed())
		{
			++m_idx;

			if (m_idx == 1)
			{
				static_assert(std::is_convertible_v<Proc2Creator,
					ProcessPtr(DataT&)>,
					"Proc2Creator must be "
					"of type DataT& -> ProcessPtr");

				m_current = m_create_proc2(m_data);

				ATP_CORE_PRECOND(m_current != nullptr);
			}
			else
			{
				m_current.reset();
			}
		}
		else if (m_current->has_failed())
		{
			// check postcondition of has_failed()
			ATP_CORE_ASSERT(m_current->done());

			m_failed = true;
			m_idx = 2;
			m_current.reset();
		}
		else
		{
			m_current->run_step();
		}
	}

private:
	size_t m_idx;
	bool m_failed;
	DataT m_data;
	ProcessPtr m_current;
	Proc2Creator m_create_proc2;
};



template<typename DataT, typename Proc1Creator,
	typename Proc2Creator>
ProcessPtr make_sequence(Proc1Creator p1, Proc2Creator p2)
{
	return std::make_shared<ProcessSequence<DataT,
		Proc1Creator, Proc2Creator>>(std::move(p1), std::move(p2));
}


}  // namespace core
}  // namespace atp


