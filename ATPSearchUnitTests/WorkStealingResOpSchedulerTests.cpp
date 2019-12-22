#include "Test.h"
#include <ResourceOperationScheduler.h>


using atpsearch::ResourceOperationSchedulerType;


struct Fixture
{
	Fixture() :
		pScheduler(atpsearch::create_resop_scheduler(ResourceOperationSchedulerType::WORK_STEALING))
	{ }

	atpsearch::ResourceOperationSchedulerPtr pScheduler;
};


BOOST_FIXTURE_TEST_SUITE(WorkStealingResourceOperationSchedulerTests, Fixture);


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
	auto id = pScheduler->next();
	BOOST_TEST(id == 7);
}


BOOST_AUTO_TEST_CASE(Test_Not_Ready_After_Calling_Next)
{
	pScheduler->add(0);
	auto id = pScheduler->next();
	BOOST_TEST(!pScheduler->ready());
}


BOOST_AUTO_TEST_CASE(Test_Concurrent_Resops)
{
	pScheduler->add(1);
	pScheduler->add(2);
	auto id1 = pScheduler->next();

	BOOST_TEST(pScheduler->ready());

	auto id2 = pScheduler->next();

	BOOST_TEST(id1 != id2);
}


BOOST_AUTO_TEST_CASE(Test_Dependent_Resops)
{
	pScheduler->add(1);
	pScheduler->add(2, 1);

	auto id1 = pScheduler->next();

	BOOST_TEST(id1 == 1);

	BOOST_TEST(!pScheduler->ready());

	pScheduler->on_finished(1);

	BOOST_TEST(pScheduler->ready());

	auto id2 = pScheduler->next();

	BOOST_TEST(id2 == 2);
}


BOOST_AUTO_TEST_SUITE_END();


