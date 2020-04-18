/**

\file

\author Samuel Barrett

\brief This suite tests `atp::logic::FreeVarIdSet`

*/


#include <Internal/FreeVarIdSet.h>
#include "Test.h"


using atp::logic::FreeVarIdSet;


struct FreeVarIdTestsFixture
{
	FreeVarIdSet set;
};


BOOST_FIXTURE_TEST_SUITE(FreeVarIdSetTests,
	FreeVarIdTestsFixture);


BOOST_DATA_TEST_CASE(basic_insert_test,
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::xrange(0, 100),
	id, test_index)
{
	set.insert(id);
	BOOST_TEST(set.contains(id));
}


BOOST_DATA_TEST_CASE(basic_insert_erase_test,
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::xrange(0, 100),
	id, test_index)
{
	set.insert(id);
	set.erase(id);
	BOOST_TEST(!set.contains(id));
}


// the insertion of two IDs can be helpful for checking the three
// cases in the insert/contains functions (see implementation for
// which three cases are being referred to)
BOOST_DATA_TEST_CASE(insert_two_test,
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::xrange(0, 100),
	id1, id2, test_index)
{
	set.insert(id1);
	set.insert(id2);

	size_t min_id = std::min(id1, id2);
	size_t max_id = std::max(id1, id2);
	size_t middle_id = (id1 + id2) / 2;

	// might wrap around, but this doesn't matter
	BOOST_TEST(!set.contains(min_id - 1));
	BOOST_TEST(!set.contains(max_id + 1));
	// may or may not contain `middle_id`, depending on if it is
	// equal to id1 or id2
	BOOST_TEST(set.contains(middle_id) ==
		(middle_id == id1 || middle_id == id2));
}


BOOST_AUTO_TEST_CASE(empty_test)
{
	BOOST_TEST(set.empty());
	BOOST_TEST(set.size() == 0);
	BOOST_TEST((set.begin() == set.end()));
}


BOOST_AUTO_TEST_CASE(many_test)
{
	size_t ids[5] =
	{
		1, 2, 3, 4, 7
	};

	for (size_t i : ids)
	{
		set.insert(i);
	}

	BOOST_TEST(set.size() == 5);
	BOOST_TEST(!set.empty());
	BOOST_TEST(std::distance(set.begin(), set.end()) == 5);

	auto iter = set.begin();
	for (size_t i = 0; i < 5; ++i, ++iter)
	{
		BOOST_TEST(*iter == ids[i]);
		BOOST_TEST(set.contains(ids[i]));
	}
}


BOOST_AUTO_TEST_CASE(max_test)
{
	size_t ids[5] =
	{
		0, 2, 3, 4, 7
	};

	for (size_t i : ids)
	{
		set.insert(i);
	}

	BOOST_TEST(set.max() == 7);
}


BOOST_AUTO_TEST_CASE(erase_out_of_range_test)
{
	set.insert(5);
	set.insert(7);
	set.erase(4);
	set.erase(6);
	set.erase(8);

	BOOST_TEST(set.contains(5));
	BOOST_TEST(set.contains(7));
	BOOST_TEST(!set.contains(4));
	BOOST_TEST(!set.contains(6));
	BOOST_TEST(!set.contains(8));
	BOOST_TEST(set.max() == 7);
	BOOST_TEST(set.size() == 2);
	BOOST_TEST(!set.empty());
}


BOOST_AUTO_TEST_CASE(test_erasing_using_iterators)
{
	set.insert(4);
	set.insert(6);

	auto iter = std::next(set.begin());

	set.erase(iter);

	BOOST_TEST(!set.contains(6));
	BOOST_TEST(set.contains(4));
	BOOST_TEST(set.size() == 1);
	BOOST_TEST(!set.empty());
	BOOST_TEST(set.max() == 4);
}


BOOST_AUTO_TEST_CASE(test_bring_forward)
{
	// in this case, the insertion of 0 forces the array to be bigger
	set.insert(0);
	set.insert(4);
	set.insert(6);
	set.erase(0);

	auto iter = std::next(set.begin());

	BOOST_TEST(*iter == 6);

	set.erase(iter);
	BOOST_TEST(!set.contains(6));
	BOOST_TEST(set.contains(4));
}


BOOST_AUTO_TEST_CASE(test_iter_equality)
{
	// in this case, the insertion of 0 forces the array to be bigger
	set.insert(0);
	set.insert(4);
	set.insert(6);
	set.erase(0);

	// test that, during equality testing, both iterators are
	// brought forward

	auto iter1 = std::next(set.begin());
	auto iter2 = std::next(set.begin());

	BOOST_TEST((iter1 == iter2));
}


BOOST_AUTO_TEST_CASE(test_remove_if)
{
	size_t ids[5] =
	{
		1, 3, 4, 6, 10
	};
	for (size_t id : ids)
		set.insert(id);

	set.remove_if([](size_t x) { return x % 2 == 0; });

	BOOST_TEST(set.contains(1));
	BOOST_TEST(set.contains(3));
	BOOST_TEST(!set.contains(4));
	BOOST_TEST(!set.contains(6));
	BOOST_TEST(!set.contains(10));
}


BOOST_AUTO_TEST_CASE(empty_set_subset_test)
{
	size_t ids[5] =
	{
		1, 3, 4, 6, 10
	};
	for (size_t id : ids)
		set.insert(id);

	FreeVarIdSet empty_set;

	BOOST_TEST(empty_set.subset(set));
	BOOST_TEST(!set.subset(empty_set));
	BOOST_TEST(empty_set.subset(empty_set));
}


BOOST_AUTO_TEST_CASE(equality_test)
{
	size_t ids[5] =
	{
		1, 3, 4, 6, 10
	};
	for (size_t id : ids)
		set.insert(id);

	// build the set differently; allocate extra size on both ends
	// of the array but then erase them
	FreeVarIdSet set2;

	set2.insert(0);
	set2.insert(1);
	set2.insert(3);
	set2.insert(4);
	set2.insert(6);
	set2.insert(10);
	set2.insert(15);
	set2.erase(0);
	set2.erase(15);

	BOOST_TEST((set == set2));
}


BOOST_AUTO_TEST_CASE(empty_set_equality_test)
{
	size_t ids[5] =
	{
		1, 3, 4, 6, 10
	};
	for (size_t id : ids)
		set.insert(id);

	FreeVarIdSet empty_set;

	BOOST_TEST((empty_set == empty_set));
	BOOST_TEST((empty_set != set));
}


BOOST_AUTO_TEST_CASE(constructor_test)
{
	set.insert(1);

	auto set2 = FreeVarIdSet(std::vector<size_t>{ 1 });

	BOOST_TEST((set == set2));
}


BOOST_AUTO_TEST_SUITE_END();


