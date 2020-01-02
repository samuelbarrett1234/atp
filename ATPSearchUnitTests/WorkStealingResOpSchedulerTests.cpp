#include "Test.h"
#include <ResourceOperationScheduler.h>


// Author: Samuel Barrett


using atpsearch::ResourceOperationSchedulerType;


struct WorkStealingResourceOperationSchedulerFixture
{
	atpsearch::ResourceOperationSchedulerPtr pScheduler = atpsearch::create_resop_scheduler(ResourceOperationSchedulerType::WORK_STEALING);
};


BOOST_FIXTURE_TEST_SUITE(WorkStealingResourceOperationSchedulerTests, WorkStealingResourceOperationSchedulerFixture);


BOOST_AUTO_TEST_CASE(Test_Not_Ready_When_No_Work)
{
	BOOST_TEST(!pScheduler->ready());
}


BOOST_AUTO_TEST_CASE(Test_Ready_When_Added_Work)
{
	pScheduler->add(0);
	BOOST_TEST(pScheduler->ready());
}


BOOST_AUTO_TEST_CASE(Test_Next_Returns_Correct_ID)
{
	pScheduler->add(7);
	auto work = pScheduler->next();
	BOOST_TEST(*work == 7);
}


BOOST_AUTO_TEST_CASE(Test_Not_Ready_After_Calling_Next)
{
	pScheduler->add(0);
	auto work = pScheduler->next();
	BOOST_TEST(!pScheduler->ready());
}


BOOST_AUTO_TEST_CASE(Test_Concurrent_Resops)
{
	pScheduler->add(1);
	pScheduler->add(2);
	auto work1 = pScheduler->next();

	BOOST_TEST(pScheduler->ready());

	auto work2 = pScheduler->next();

	BOOST_TEST(*work1 != *work2);
}


BOOST_AUTO_TEST_CASE(Test_Dependent_Resops)
{
	pScheduler->add(1);
	pScheduler->add(2, 1);

	{
		auto work1 = pScheduler->next();

		BOOST_TEST(*work1 == 1);

		BOOST_TEST(!pScheduler->ready());

	}  // work1 destructs and calls on_finished()
	
	BOOST_TEST(pScheduler->ready());

	auto work2 = pScheduler->next();

	BOOST_TEST(*work2 == 2);
}


BOOST_AUTO_TEST_SUITE_END();


