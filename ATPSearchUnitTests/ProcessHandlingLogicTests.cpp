#include "Test.h"
#include <ProcessHandlingLogic.h>


using atpsearch::ProcessSchedulerType;
using atpsearch::ResourceOperationSchedulerType;


struct ProcessHandlingLogicFixture
{
	ProcessHandlingLogicFixture() :
		proc_handler(
			ProcessSchedulerType::WORK_STEALING,
			ResourceOperationSchedulerType::WORK_STEALING
		)
	{ }

	atpsearch::ProcessHandlingLogic proc_handler;
};


BOOST_FIXTURE_TEST_SUITE(ProcessHandlingLogicTests, ProcessHandlingLogicFixture);


BOOST_AUTO_TEST_CASE(Test_Add_Worker_Then_Next_Returns_It)
{
	proc_handler.add_new_process(7U);
	size_t id;
	BOOST_TEST(proc_handler.next_worker_id_if_exists(&id));
	BOOST_TEST(id == 7U);
}


BOOST_AUTO_TEST_CASE(Test_No_Workers_By_Default)
{
	BOOST_TEST(!proc_handler.next_worker_id_if_exists(nullptr));
}


BOOST_AUTO_TEST_CASE(Test_Next_Doesnt_Return_Same_ID_Twice)
{
	proc_handler.add_new_process(8U);
	size_t id;
	proc_handler.next_worker_id_if_exists(&id);
	BOOST_TEST(!proc_handler.next_worker_id_if_exists(&id));
}


BOOST_AUTO_TEST_CASE(Test_Next_With_No_Out_Arg_Is_Const)
{
	proc_handler.add_new_process(9U);
	size_t id;
	proc_handler.next_worker_id_if_exists(nullptr);
	BOOST_TEST(proc_handler.next_worker_id_if_exists(&id));
	BOOST_TEST(id == 9U);
}


BOOST_AUTO_TEST_SUITE_END();


