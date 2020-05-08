#pragma once


/**
\file

\author Samuel Barrett

\brief Contains templated classes to make it easier to create
	lists of processes which run in sequence.

*/


#include <type_traits>
#include <functional>
#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>
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
template<typename DataTuple,
	typename ProcCreatorTuple>
class ProcessSequence :
	public IProcess
{
public:
	ProcessSequence(ProcCreatorTuple funcs) :
		m_proc_creators(std::move(funcs)),
		m_idx(0), m_failed(false)
	{
		create_proc_at<0>();

		static_assert(SIZE ==
			boost::tuples::length<DataTuple>::value,
			"Number of process functions must be equal to "
			"the number of data items!");
	}

	inline bool done() const override
	{
		return m_idx == SIZE;
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

			// create the next process

			m_current.reset();

			if (m_idx < SIZE)
			{
				// sprinkle a bit of template magic to utilise the fact
				// that we know 0 <= m_idx < SIZE, so we can think of
				// unrolling a for-loop to have the effect of calling:
				// create_proc_at<m_idx>() but of course the latter would
				// not be allowed.
				ProcCreator<SIZE - 1> creator(*this);
				creator(m_idx);
			}
			// else we are done
		}
		else if (m_current->has_failed())
		{
			// check postcondition of has_failed()
			ATP_CORE_ASSERT(m_current->done());

			// we have failed, so just "jump to the end"
			m_failed = true;
			m_idx = SIZE;
			m_current.reset();
		}
		else
		{
			// this is the normal case; just execute the current
			// process for one step.
			m_current->run_step();
		}
	}

	template<size_t i>
	inline auto& get_data()
	{
		static_assert(i < SIZE,
			"Data index out of range.");

		return m_datas.get<i>();
	}

private:
	template<size_t i>
	void create_proc_at()
	{
		ATP_CORE_PRECOND(!m_failed);
		ATP_CORE_PRECOND(m_idx == i);

		static_assert(i < SIZE,
			"i out of bounds.");

		if constexpr (i == 0)
		{
			static_assert(std::is_convertible_v<
				decltype(m_proc_creators.get<0>()),
				std::function<ProcessPtr(
				decltype(m_datas.get<0>()))>>,
				"Process creator function has wrong type.");

			// handle first process as a special case
			m_current = m_proc_creators.get<0>()(
				m_datas.get<0>());
		}
		else
		{
			static_assert(std::is_convertible_v<
				decltype(m_proc_creators.get<i>()),
				std::function<ProcessPtr(
					decltype(m_datas.get<i - 1>()),
					decltype(m_datas.get<i>()))>>,
				"Process creator function has wrong type.");

			m_current = m_proc_creators.get<i>()(
				m_datas.get<i - 1>(), m_datas.get<i>());
		}

		ATP_CORE_PRECOND(m_current != nullptr);
	}

	template<size_t i>
	struct ProcCreator
	{
		ProcessSequence<DataTuple, ProcCreatorTuple>& parent;
		ProcCreator<i - 1> child;

		ProcCreator(ProcessSequence<DataTuple,
			ProcCreatorTuple>& parent) :
			parent(parent), child(parent)
		{ }

		void operator()(size_t j)
		{
			if (i == j)
				parent.create_proc_at<i>();
			else
				child(j);
		}
	};
	template<>
	struct ProcCreator<0>
	{
		ProcessSequence<DataTuple, ProcCreatorTuple>& parent;

		ProcCreator(ProcessSequence<DataTuple,
			ProcCreatorTuple>& parent) :
			parent(parent)
		{ }

		void operator()(size_t j)
		{
			ATP_CORE_ASSERT(j == 0);
			parent.create_proc_at<0>();
		}
	};

private:
	static constexpr const size_t SIZE =
		boost::tuples::length<ProcCreatorTuple>::value;
	size_t m_idx;
	bool m_failed;
	ProcessPtr m_current;
	DataTuple m_datas;
	ProcCreatorTuple m_proc_creators;
};


template<typename ... DataItems,
	typename ... ProcCreatorFuncs>
std::shared_ptr<ProcessSequence<
	boost::tuple<DataItems...>,
	boost::tuple<ProcCreatorFuncs...>>>
	make_sequence(
	boost::tuple<ProcCreatorFuncs...> funcs)
{
	return std::make_shared<ProcessSequence<
		boost::tuple<DataItems...>,
		boost::tuple<ProcCreatorFuncs...>>>(
			std::move(funcs));
}


}  // namespace core
}  // namespace atp


