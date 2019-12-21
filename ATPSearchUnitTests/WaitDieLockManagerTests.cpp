#include "Test.h"
#include <LockManager.h>


using atpsearch::LockManagementType;
using atpsearch::WorkerStatus;
using atpsearch::LockType;


struct Fixture
{
	atpsearch::LockManagerPtr pLkMgr = atpsearch::create_lock_manager(LockManagementType::WAIT_DIE);
	WorkerStatus status;
};


BOOST_FIXTURE_TEST_SUITE(WaitDieLockManagerTests, Fixture);


BOOST_AUTO_TEST_CASE(Test_Wait_On_XLock_Request)
{
	pLkMgr->request(1, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->request(0, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::BLOCKED);
}


BOOST_AUTO_TEST_CASE(Test_Die_On_XLock_Request)
{
	pLkMgr->request(0, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->request(1, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::FAILED);
}


BOOST_AUTO_TEST_CASE(Test_Remove_Unlocks_XLock)
{
	pLkMgr->request(0, 0, LockType::XLOCK);

	pLkMgr->remove_worker(0);

	pLkMgr->request(1, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	// 0 should not be blocked, too.
	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);
}


BOOST_AUTO_TEST_CASE(Test_XLock_On_Distinct_Resources_Is_Successful)
{
	pLkMgr->request(0, 0, LockType::XLOCK);
	pLkMgr->request(1, 1, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);
}


BOOST_AUTO_TEST_CASE(Test_Multiple_SLock_Passes)
{
	pLkMgr->request(0, 0, LockType::SLOCK);
	pLkMgr->request(1, 0, LockType::SLOCK);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);
}


BOOST_AUTO_TEST_CASE(Test_SLock_Blocks_XLock_With_Wait)
{
	//Wait-die logic still applies here

	pLkMgr->request(0, 0, LockType::SLOCK);
	pLkMgr->request(1, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::FAILED);
}


BOOST_AUTO_TEST_CASE(Test_SLock_Blocks_XLock_With_Die)
{
	//Wait-die logic still applies here

	pLkMgr->request(1, 0, LockType::SLOCK);
	pLkMgr->request(0, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::BLOCKED);
}


BOOST_AUTO_TEST_CASE(Test_XLock_Blocks_SLock_With_Wait)
{
	//Wait-die logic still applies here

	pLkMgr->request(0, 0, LockType::XLOCK);
	pLkMgr->request(1, 0, LockType::SLOCK);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::FAILED);
}


BOOST_AUTO_TEST_CASE(Test_XLock_Blocks_SLock_With_Die)
{
	//Wait-die logic still applies here

	pLkMgr->request(1, 0, LockType::XLOCK);
	pLkMgr->request(0, 0, LockType::SLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::BLOCKED);
}


BOOST_AUTO_TEST_CASE(Test_Remove_Unlocks_XLock_2)
{
	pLkMgr->request(0, 0, LockType::XLOCK);

	pLkMgr->remove_worker(0);

	pLkMgr->request(1, 0, LockType::SLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	// 0 should not be blocked, too.
	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);
}


BOOST_AUTO_TEST_CASE(Test_Remove_Unlocks_SLock)
{
	pLkMgr->request(0, 0, LockType::SLOCK);

	pLkMgr->remove_worker(0);

	pLkMgr->request(1, 0, LockType::XLOCK);

	pLkMgr->get_status_freeze_if_ready(1, &status);
	BOOST_TEST(status == WorkerStatus::READY);

	// 0 should not be blocked, too.
	pLkMgr->get_status_freeze_if_ready(0, &status);
	BOOST_TEST(status == WorkerStatus::READY);
}


BOOST_AUTO_TEST_SUITE_END();


