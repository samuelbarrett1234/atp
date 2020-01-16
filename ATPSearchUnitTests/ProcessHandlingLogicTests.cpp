#include "Test.h"
#include <ProcessHandlingLogic.h>


// Author: Samuel Barrett


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


BOOST_AUTO_TEST_CASE(Test_Add_Worker_Then_Next_Returns_Its_ID)
{
	proc_handler.add_new_process(7U);

	auto _data = proc_handler.try_begin_next_process();
	auto& data = _data.get();

	BOOST_TEST(data.process_id == 7U);
}


BOOST_AUTO_TEST_CASE(Test_Returned_Data_Says_Process_Is_Not_Finished)
{
	proc_handler.add_new_process(7U);

	auto _data = proc_handler.try_begin_next_process();
	auto& data = _data.get();

	BOOST_TEST(!data.b_finished);
}


BOOST_AUTO_TEST_CASE(Test_Returned_Data_Contains_No_ResOps)
{
	proc_handler.add_new_process(7U);

	auto _data = proc_handler.try_begin_next_process();
	auto& data = _data.get();

	BOOST_TEST(data.res_ops.empty());
}


BOOST_AUTO_TEST_CASE(Test_No_Workers_By_Default)
{
	BOOST_TEST(!proc_handler.try_begin_next_process().has_value());
}


BOOST_AUTO_TEST_CASE(Test_Next_Returns_Data_Then_Empty_When_Called_Twice_But_One_Data_Available)
{
	proc_handler.add_new_process(8U);

	auto data1 = proc_handler.try_begin_next_process();

	BOOST_TEST(data1.has_value());
	
	auto data2 = proc_handler.try_begin_next_process();

	BOOST_TEST(!data2.has_value());
}


BOOST_AUTO_TEST_CASE(Test_Making_Process_Finished_Stops_It_Being_Returned_Again)
{
	proc_handler.add_new_process(2U);

	{
		auto data = proc_handler.try_begin_next_process();
		
		data.get().b_finished = true;  // should trigger it on data's destructor
	}

	BOOST_TEST(!proc_handler.try_begin_next_process().has_value());
}


BOOST_AUTO_TEST_CASE(Test_Data_Destructor_Readds_It_To_Pool_If_No_ResOps)
{
	proc_handler.add_new_process(3U);

	proc_handler.try_begin_next_process();  // calls destructor immediately so should re-add it to pool

	auto data = proc_handler.try_begin_next_process();  // should return the same as the line above

	BOOST_TEST(data.get().process_id == 3U);
}


BOOST_AUTO_TEST_CASE(Test_No_ResOps_When_Empty)
{
	BOOST_TEST(!proc_handler.try_begin_next_res_op().has_value());
}


BOOST_AUTO_TEST_CASE(Test_Getting_ResOp)
{
	proc_handler.add_new_process(7U);

	{
		auto data = proc_handler.try_begin_next_process();

		// This res-op should get added when data is destructed
		data.get().res_ops.push_back(
			atpsearch::resop::pipe(0U, 1U, 1024U, nullptr)
		);
	}

	auto resop_data = proc_handler.try_begin_next_res_op();
	auto res_op = resop_data.get().res_op;
	
	// Check this data struct has identified the right resop:
	BOOST_TEST(resop_data.get().process_id == 7U);
	BOOST_TEST(resop_data.get().res_op_idx == 0U);

	// This should always start as false:
	BOOST_TEST(!resop_data.get().b_failed);

	// Check that the returned resource operation contains all
	// the correct details:
	BOOST_TEST(res_op.type == atpsearch::ResourceOperation::Type::PIPE);
	BOOST_TEST(res_op.pipe.source == 0U);
	BOOST_TEST(res_op.pipe.target == 1U);
	BOOST_TEST(res_op.pipe.transferSize == 1024U);
	BOOST_TEST(res_op.pipe.amountTransferred == nullptr);
}


BOOST_AUTO_TEST_CASE(Test_Getting_ResOp_Twice_When_One_Available)
{
	proc_handler.add_new_process(7U);

	{
		auto data = proc_handler.try_begin_next_process();

		// This res-op should get added when data is destructed
		data.get().res_ops.push_back(
			atpsearch::resop::pipe(0U, 1U, 1024U, nullptr)
		);
	}

	auto resop_data1 = proc_handler.try_begin_next_res_op();

	// resop_data should contain the data

	auto resop_data_2 = proc_handler.try_begin_next_res_op();

	BOOST_TEST(!resop_data_2.has_value());  // should be empty
}


BOOST_AUTO_TEST_CASE(Test_Getting_Two_Resops_At_Once_Which_Dont_Depend)
{
	proc_handler.add_new_process(7U);

	{
		auto data = proc_handler.try_begin_next_process();

		// This res-op should get added when data is destructed
		data.get().res_ops.push_back(
			atpsearch::resop::pipe(0U, 1U, 1024U, nullptr)
		);
		data.get().res_ops.push_back(
			atpsearch::resop::pipe(2U, 3U, 1024U, nullptr)
		);
	}

	auto resop_data_1 = proc_handler.try_begin_next_res_op();
	auto resop_data_2 = proc_handler.try_begin_next_res_op();

	auto resop_1 = resop_data_1.get().res_op;
	auto resop_2 = resop_data_2.get().res_op;

	// Now, technically the order of resop_1 and resop_2 shouldn't matter,
	// so we need to be careful in the way we test them:

	BOOST_TEST(resop_1.pipe.source != resop_2.pipe.source);  // this is sufficient to discern the two
}


BOOST_AUTO_TEST_CASE(Test_Getting_Two_Resops_At_Once_Which_Do_Depend)
{
	proc_handler.add_new_process(7U);

	{
		auto data = proc_handler.try_begin_next_process();

		// This res-op should get added when data is destructed
		data.get().res_ops.push_back(
			atpsearch::resop::pipe(0U, 1U, 1024U, nullptr)
		);
		data.get().res_ops.push_back(
			atpsearch::resop::pipe(2U, 3U, 1024U, nullptr, 0U)
		);
	}

	{
		auto resop_data_1 = proc_handler.try_begin_next_res_op();
		auto resop_data_2 = proc_handler.try_begin_next_res_op();

		BOOST_TEST(!resop_data_2.has_value());

		// Now it is important to check the source/target of
		// resop_1 to ensure it agrees with the order we specified
		// above:

		BOOST_TEST(resop_data_1.get().res_op.pipe.source == 0U);
		BOOST_TEST(resop_data_1.get().res_op.pipe.target == 1U);
	}

	{
		// Now we should be able to get the next one!

		auto resop_data_2 = proc_handler.try_begin_next_res_op();

		// Now it is important to check the source/target of
		// resop_2 to ensure it agrees with the order we specified
		// above:

		BOOST_TEST(resop_data_2.get().res_op.pipe.source == 2U);
		BOOST_TEST(resop_data_2.get().res_op.pipe.target == 3U);
	}
}


// IMPORTANT: make sure we write plenty of unit tests for failure!


BOOST_AUTO_TEST_SUITE_END();


