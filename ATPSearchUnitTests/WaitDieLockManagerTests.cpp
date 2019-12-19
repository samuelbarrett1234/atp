#include "Test.h"
#include <LockManager.h>


using atpsearch::LockManagementType;
using atpsearch::WorkerStatus;
using atpsearch::LockType;


struct Fixture
{
	atpsearch::LockManagerPtr pLkMgr = atpsearch::create_lock_manager(LockManagementType::WAIT_DIE);
};


BOOST_FIXTURE_TEST_SUITE(WaitDieLockManagerTests, Fixture);


BOOST_AUTO_TEST_CASE(Test_Wait_On_XLock_Request)
{
	BOOST_TEST(pLkMgr->request(1, 0, LockType::XLOCK) == LockRequestResult::PASSED);

	BOOST_TEST(pLkMgr->request(0, 0, LockType::XLOCK) == LockRequestResult::PASSED);
	
	BOOST_TEST(pLkMgr->is_blocked(0));
	BOOST_TEST(!pLkMgr->is_blocked(1));
}


BOOST_AUTO_TEST_CASE(Test_Die_On_XLock_Request)
{
	BOOST_TEST(pLkMgr->request(0, 0, LockType::XLOCK) == LockRequestResult::PASSED);

	BOOST_TEST(pLkMgr->request(1, 0, LockType::XLOCK) == LockRequestResult::FAILED);

	BOOST_TEST(!pLkMgr->is_blocked(0));
	BOOST_TEST(!pLkMgr->is_blocked(1));
}


BOOST_AUTO_TEST_CASE(Test_Remove_Unlocks_XLock)
{
	BOOST_TEST(pLkMgr->request(0, 0, LockType::XLOCK) == LockRequestResult::PASSED);

	pLkMgr->remove_worker(0);

	BOOST_TEST(pLkMgr->request(1, 0, LockType::XLOCK) == LockRequestResult::PASSED);

	BOOST_TEST(!pLkMgr->is_blocked(0));
	BOOST_TEST(!pLkMgr->is_blocked(1));
}


BOOST_AUTO_TEST_CASE(Test_XLock_On_Distinct_Resources_Is_Successful)
{
	BOOST_TEST(pLkMgr->request(0, 0, LockType::XLOCK) == LockRequestResult::PASSED);
	BOOST_TEST(pLkMgr->request(1, 1, LockType::XLOCK) == LockRequestResult::PASSED);

	BOOST_TEST(!pLkMgr->is_blocked(0));
	BOOST_TEST(!pLkMgr->is_blocked(1));
}


BOOST_AUTO_TEST_SUITE_END();


