/**
\file

\author Samuel Barrett

\brief This suite tests the ProcessSequence functionality, for
	chaining together lots of processes at compile time. This is more
	a compilation test, than anything else.

*/


#include <boost/bind.hpp>
#include "Processes/IProcess.h"
#include "Processes/ProcessSequence.h"
#include "Test.h"


using atp::core::IProcess;
using atp::core::ProcessSequence;
using atp::core::make_sequence;
using atp::core::ProcessPtr;


template<typename DataT>
class TestProcess1 :
	public IProcess
{
public:
	static ProcessPtr make(DataT& dt)
	{
		return std::make_shared<TestProcess1<DataT>>(dt);
	}

	TestProcess1(DataT& dt) :
		m_data(dt)
	{ }

	inline bool done() const override
	{
		return m_done;
	}
	inline bool has_failed() const override
	{
		return m_failed;
	}
	inline bool waiting() const override
	{
		return m_waiting;
	}
	inline void run_step() override
	{
		++m_num_steps;
		m_done = true;
	}

	bool m_done = false, m_waiting = false, m_failed = false;
	size_t m_num_steps = 0;
	DataT m_data;
};


template<typename Data1T, typename Data2T>
class TestProcess2 :
	public IProcess
{
public:
	static ProcessPtr make(Data1T& dt1, Data2T& dt2)
	{
		return std::make_shared<TestProcess2<Data1T,
			Data2T>>(dt1, dt2);
	}

	TestProcess2(Data1T& dt1, Data2T& dt2) :
		m_data1(dt1), m_data2(dt2)
	{ }

	inline bool done() const override
	{
		return m_done;
	}
	inline bool has_failed() const override
	{
		return m_failed;
	}
	inline bool waiting() const override
	{
		return m_waiting;
	}
	inline void run_step() override
	{
		++m_num_steps;
		m_done = true;
	}

	bool m_done = false, m_waiting = false, m_failed = false;
	size_t m_num_steps = 0;
	Data1T m_data1;
	Data2T m_data2;
};


BOOST_AUTO_TEST_SUITE(ProcessSequenceTests);


BOOST_AUTO_TEST_CASE(test_put_together_process_sequence)
{
	bool data1 = false;
	size_t data2 = 7;
	std::string data3 = "test";

	auto p_proc = make_sequence<
		bool, size_t, std::string>(boost::make_tuple(
		boost::bind(&TestProcess2<bool, size_t>::make, _1, _2),
		boost::bind(&TestProcess2<size_t, std::string>::make, _1, _2),
		boost::bind(&TestProcess1<std::string>::make, _1)
	));

	BOOST_REQUIRE(p_proc != nullptr);
	BOOST_TEST(!p_proc->done());
	BOOST_TEST(!p_proc->waiting());
	BOOST_TEST(!p_proc->has_failed());

	// count the number of iterations until the process terminates,
	// which should be exactly N by the way we set it up.
	static const size_t N = 6;
	size_t i = 0;
	for (; i < N &&
		!p_proc->done(); ++i)
	{
		p_proc->run_step();
	}

	BOOST_TEST(i == N);
	BOOST_TEST(p_proc->done());
}


BOOST_AUTO_TEST_SUITE_END();


