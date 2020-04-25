/**
\file

\author Samuel Barrett

\brief Test suite for the StrictLockManager

\note It is understood that the strict lock manager does not
	distinguish between read-only and read-write locks, thus
	we treat all three of its `request` functions uniformly
	and interchange between them as we wish.
*/


#include <vector>
#include <random>
#include <Internal/StrictLockManager.h>
#include "Test.h"


using atp::db::StrictLockManager;
using atp::db::LockPtr;
using atp::db::ResourceName;
using atp::db::ResourceList;


struct StrictLockManagerTestsFixture
{
	StrictLockManager lkmgr;
};


BOOST_AUTO_TEST_SUITE(LockManagerTests);
BOOST_FIXTURE_TEST_SUITE(StrictLockManagerTests,
	StrictLockManagerTestsFixture);


BOOST_AUTO_TEST_CASE(test_livelihood)
{
	// test that it can actually give out locks when they are free

	auto lock1 = lkmgr.request_write_access({ 1 });
	
	BOOST_TEST(lock1 != nullptr);

	auto lock2 = lkmgr.request_write_access({ 2 });

	BOOST_TEST(lock2 != nullptr);
}


BOOST_AUTO_TEST_CASE(test_exclusion)
{
	auto lock1 = lkmgr.request_write_access({ 1 });
	auto lock2 = lkmgr.request_write_access({ 1 });

	BOOST_TEST(lock2 == nullptr);
}


BOOST_AUTO_TEST_CASE(test_lock_release)
{
	{
		auto lock1 = lkmgr.request_write_access({ 1 });
	
	}  // lock destroyed here

	auto lock2 = lkmgr.request_write_access({ 1 });

	BOOST_TEST(lock2 != nullptr);
}


BOOST_AUTO_TEST_CASE(overlapping_resources_test)
{
	auto lock1 = lkmgr.request_write_access({ 1, 2 });
	auto lock2 = lkmgr.request_write_access({ 2, 3 });

	BOOST_TEST(lock1 != nullptr);
	BOOST_TEST(lock2 == nullptr);
}


BOOST_DATA_TEST_CASE(randomised_test,
	boost::unit_test::data::xrange(250) ^
	boost::unit_test::data::random(1, 10) ^
	boost::unit_test::data::random(1, 20) ^
	boost::unit_test::data::random(10, 1000),
	test_idx, num_resources, num_threads, num_timesteps)
{
	std::random_device dev;
	std::uniform_int_distribution<size_t> thread_dist(0,
		num_threads - 1);
	std::bernoulli_distribution ber_dist(0.2);

	// perform a load of randomised operations, and track separately
	// that none of the locking is violated.

	std::vector<bool> res_locked;  // whether each resource locked
	res_locked.resize(num_resources, false);

	// each thread may hold a lock
	std::vector<LockPtr> thread_locks;
	thread_locks.resize(num_threads);

	for (size_t t = 0; t < num_timesteps; ++t)
	{
		// pick a thread at random
		const size_t thread = thread_dist(dev);

		// if we already have a lock then release it
		if (thread_locks[thread] != nullptr)
		{
			// mark all the locked resources as now unlocked
			auto locked_res = thread_locks[thread]->
				get_locked_resources();
			for (auto res : locked_res)
			{
				BOOST_TEST(res_locked[res]);
				res_locked[res] = false;
			}

			// destroy the lock
			thread_locks[thread].reset();
		}
		// else try to obtain a lock for a random set of resources
		else
		{
			// generate a list of resources to try locking
			ResourceList res_request;
			for (size_t i = 0; i < num_resources; ++i)
			{
				if (ber_dist(dev))
				{
					res_request.push_back(i);
				}
			}

			// try to obtain a lock
			thread_locks[thread] = lkmgr.request_write_access(
				res_request);

			// check that the result was correct
			if (thread_locks[thread] != nullptr)
			{
				// we got a lock; check that none of them were
				// locked already
				for (auto res : res_request)
				{
					BOOST_TEST(!res_locked[res]);
					res_locked[res] = true;
				}
			}
			else
			{
				// we didn't get a lock; check that at least one of
				// the resources we requested was already locked.
				BOOST_TEST(std::any_of(res_request.begin(),
					res_request.end(),
					[&res_locked](size_t i)
					{ return res_locked[i]; }));
			}
		}
	}
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


