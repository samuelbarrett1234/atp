#include "Test.h"
#include "DummyProcess.h"
#include <ProcessScheduler.h>


// Author: Samuel Barrett


using atpsearch::ProcessSchedulerType;
using atpsearch::ProcessPtr;


struct WorkStealingProcessSchedulerFixture
{
	atpsearch::ProcessSchedulerPtr pScheduler = atpsearch::create_process_scheduler(ProcessSchedulerType::WORK_STEALING);
};


BOOST_FIXTURE_TEST_SUITE(WorkStealingProcessSchedulerTests, WorkStealingProcessSchedulerFixture);


BOOST_AUTO_TEST_CASE(Test_PushToOneThread_PopFromAnotherThread)
{
	pScheduler->push(std::make_unique<DummyProcess>(), 0U);
	auto pProc = pScheduler->pop(1U);
	BOOST_TEST(pProc.get() != nullptr);
}


BOOST_AUTO_TEST_CASE(Test_Empty_Pop_Returns_Nullptr)
{
	BOOST_TEST(pScheduler->pop(1234U).get() == nullptr);
}


BOOST_AUTO_TEST_SUITE_END();


