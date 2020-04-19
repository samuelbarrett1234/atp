/**

\file

\author Samuel Barrett

\brief This suite tests `atp::logic::FreeVarMap`

*/


#include <Internal/FreeVarMap.h>
#include "Test.h"


using atp::logic::FreeVarMap;


struct FreeVarMapTestsFixture
{
	FreeVarMap<int> map;
};


BOOST_FIXTURE_TEST_SUITE(FreeVarMapTests,
	FreeVarMapTestsFixture);


BOOST_DATA_TEST_CASE(basic_insert_test,
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::random(-100, 100) ^
	boost::unit_test::data::xrange(0, 100),
	id, val, test_index)
{
	map.insert(id, val);
	BOOST_TEST(map.contains(id));
	BOOST_TEST(map.at(id) == val);
}


BOOST_DATA_TEST_CASE(basic_insert_erase_test,
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::random(-100, 100) ^
	boost::unit_test::data::xrange(0, 100),
	id, val, test_index)
{
	map.insert(id, val);
	map.erase(id);
	BOOST_TEST(!map.contains(id));
}


// the insertion of two IDs can be helpful for checking the three
// cases in the insert/contains functions (see implementation for
// which three cases are being referred to)
BOOST_DATA_TEST_CASE(insert_two_test,
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::random(0, 10) ^
	boost::unit_test::data::random(-100, 100) ^
	boost::unit_test::data::random(-100, 100) ^
	boost::unit_test::data::xrange(0, 200),
	id1, id2, val1, val2, test_index)
{
	map.insert(id1, val1);
	map.insert(id2, val2);

	size_t min_id = std::min(id1, id2);
	size_t max_id = std::max(id1, id2);
	size_t middle_id = (id1 + id2) / 2;

	// might wrap around, but this doesn't matter
	BOOST_TEST(!map.contains(min_id - 1));
	BOOST_TEST(!map.contains(max_id + 1));
	// may or may not contain `middle_id`, depending on if it is
	// equal to id1 or id2
	BOOST_TEST(map.contains(middle_id) ==
		(middle_id == id1 || middle_id == id2));

	// mimick `contains` tests for `find`
	BOOST_TEST((map.find(min_id - 1) == map.end()));
	BOOST_TEST((map.find(max_id + 1) == map.end()));
	BOOST_TEST((map.find(middle_id) != map.end()) ==
		(middle_id == id1 || middle_id == id2));

	if (id1 != id2)
	{
		// check values as well
		BOOST_TEST(map.at(id1) == val1);
		BOOST_TEST(map.at(id2) == val2);
		BOOST_TEST(map.find(id1).first() == id1);
		BOOST_TEST(map.find(id1).second() == val1);
		BOOST_TEST(map.find(id2).first() == id2);
		BOOST_TEST(map.find(id2).second() == val2);
	}
	else
	{
		// latter value should've overridden
		BOOST_TEST(map.find(id1).first() == id1);
		BOOST_TEST(map.find(id1).second() == val2);
	}
}


BOOST_AUTO_TEST_CASE(empty_test)
{
	BOOST_TEST(map.empty());
	BOOST_TEST(map.size() == 0);
	BOOST_TEST((map.begin() == map.end()));
}


BOOST_AUTO_TEST_CASE(many_test)
{
	size_t ids[5] =
	{
		1, 2, 3, 4, 7
	};
	int vals[5] =
	{
		-5, 1, 0, 200000, 42
	};

	for (size_t i = 0; i < 5; ++i)
	{
		map.insert(ids[i], vals[i]);
	}

	BOOST_TEST(map.size() == 5);
	BOOST_TEST(!map.empty());
	BOOST_TEST(std::distance(map.begin(), map.end()) == 5);

	auto iter = map.begin();
	for (size_t i = 0; i < 5; ++i, ++iter)
	{
		BOOST_TEST(*iter == vals[i]);
		BOOST_TEST(iter.first() == ids[i]);
		BOOST_TEST(iter.second() == vals[i]);
		BOOST_TEST(map.contains(ids[i]));
		BOOST_TEST((map.find(ids[i]) == iter));
		BOOST_TEST(map.at(ids[i]) == vals[i]);
	}
}


BOOST_AUTO_TEST_CASE(erase_out_of_range_test)
{
	map.insert(5, -1);
	map.insert(7, 0);
	map.erase(4);
	map.erase(6);
	map.erase(8);

	BOOST_TEST(map.contains(5));
	BOOST_TEST(map.contains(7));
	BOOST_TEST(!map.contains(4));
	BOOST_TEST(!map.contains(6));
	BOOST_TEST(!map.contains(8));
	BOOST_TEST(map.size() == 2);
	BOOST_TEST(!map.empty());
	BOOST_TEST(map.at(5) == -1);
	BOOST_TEST(map.at(7) == 0);
}


BOOST_AUTO_TEST_CASE(test_erasing_using_iterators)
{
	map.insert(4, -1);
	map.insert(6, -42);

	auto iter = std::next(map.begin());

	map.erase(iter);

	BOOST_TEST(!map.contains(6));
	BOOST_TEST(map.contains(4));
	BOOST_TEST(map.size() == 1);
	BOOST_TEST(!map.empty());
	BOOST_TEST(map.at(4) == -1);
}


BOOST_AUTO_TEST_CASE(test_bring_forward)
{
	// in this case, the insertion of 0 forces the array to be bigger
	map.insert(0, -1);
	map.insert(4, -2);
	map.insert(6, -3);
	map.erase(0);

	auto iter = std::next(map.begin());

	BOOST_TEST(*iter == -3);
	BOOST_TEST(iter.first() == 6);
	BOOST_TEST(iter.second() == -3);

	map.erase(iter);
	BOOST_TEST(!map.contains(6));
	BOOST_TEST(map.contains(4));
	BOOST_TEST(map.at(4) == -2);
}


BOOST_AUTO_TEST_CASE(test_iter_equality)
{
	// in this case, the insertion of 0 forces the array to be bigger
	map.insert(0, -1);
	map.insert(4, -2);
	map.insert(6, -3);
	map.erase(0);

	// test that, during equality testing, both iterators are
	// brought forward

	auto iter1 = std::next(map.begin());
	auto iter2 = std::next(map.begin());

	BOOST_TEST((iter1 == iter2));
}


BOOST_AUTO_TEST_CASE(test_with_nontrivial_objects)
{
	// this is more of a compile-time test than anything else

	FreeVarMap<std::vector<size_t>> map2;

	const std::vector<size_t> pass_me_by_const_ref = { 1, 2, 3 };
	std::vector<size_t> move_me = { 4, 5, 6 };

	map2.insert(0, pass_me_by_const_ref);
	map2.insert(1, std::move(move_me));

	BOOST_TEST((map2.at(0) == pass_me_by_const_ref));
	BOOST_TEST((map2.at(1) == std::vector<size_t>{ 4, 5, 6 }));
}


BOOST_AUTO_TEST_CASE(overwrite_test)
{
	map.insert(0, -1);
	map.insert(0, -2);
	BOOST_TEST(map.at(0) == -2);
}


BOOST_AUTO_TEST_CASE(clear_test)
{
	map.insert(0, 7);
	map.insert(1, -4);
	map.clear();
	BOOST_TEST(map.empty());
	BOOST_TEST(map.size() == 0);
	BOOST_TEST(!map.contains(0));
	BOOST_TEST(!map.contains(1));
}


BOOST_AUTO_TEST_CASE(test_modifiable_values)
{
	map.insert(0, -1);
	map.at(0) = -2;
	BOOST_TEST(map.at(0) == -2);
	auto iter = map.begin();
	*iter = -3;
	BOOST_TEST(map.at(0) == -3);
	iter.second() = -4;
	BOOST_TEST(map.at(0) == -4);
}


BOOST_AUTO_TEST_CASE(test_iterator_first)
{
	map.insert(1, -1);
	BOOST_TEST(map.begin().first() == 1);
}


BOOST_AUTO_TEST_CASE(test_iterator_second)
{
	map.insert(1, -1);
	BOOST_TEST(map.begin().second() == -1);
}


BOOST_AUTO_TEST_CASE(test_iteration_after_clear)
{
	map.insert(0, -1);
	map.insert(1, -2);

	map.clear();

	map.insert(1, -3);

	// a load of random tests:

	BOOST_TEST(map.size() == 1);
	BOOST_TEST(std::any_of(map.begin(), map.end(),
		[](int x) { return x == -3; }));
	BOOST_TEST(std::distance(map.begin(), map.end()) == 1);

	BOOST_TEST(map.begin().first() == 1);

	for (auto iter = map.begin(); iter != map.end(); ++iter)
	{
		BOOST_TEST(iter.first() == 1);
		BOOST_TEST(iter.second() == -3);
	}

	BOOST_TEST(!map.contains(0));
}


BOOST_AUTO_TEST_CASE(test_iterator_brought_forward_in_eq_test)
{
	// causes m_vec to be of size 1
	map.insert(1, -1);
	map.erase(1);

	BOOST_TEST((map.begin() == map.end()));
}




// TODO: implement this functionality in FreeVarMap??
//BOOST_AUTO_TEST_CASE(insert_via_at_test)
//{
//	map.at(0) = -5;
//	BOOST_TEST(map.at(0) == -5);
//}


BOOST_AUTO_TEST_SUITE_END();


