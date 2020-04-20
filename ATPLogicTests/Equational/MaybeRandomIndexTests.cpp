/**
\file

\author Samuel Barrett

\brief This suite tests the `MaybeRandomIndex` class for iterating
	over arrays in either a random or normal fashion.
*/


#include <vector>
#include <Internal/Equational/MaybeRandomIndex.h>
#include "../Test.h"


using atp::logic::equational::MaybeRandomIndex;


struct MaybeRandomIndexTestsFixture
{
	std::vector<bool> vec;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(MaybeRandomIndexTests,
	MaybeRandomIndexTestsFixture);


BOOST_AUTO_TEST_CASE(basic_test)
{
	const size_t N = 100;
	vec.resize(N, false);
	MaybeRandomIndex idx(N, false);
	for (size_t i = 0; i < N; ++i, idx.advance())
	{
		size_t j = idx.get();
		BOOST_REQUIRE(j < vec.size());
		BOOST_TEST(!vec[j]);
		vec[j] = true;
	}
	BOOST_TEST(idx.is_end());
	BOOST_TEST(std::all_of(vec.begin(), vec.end(),
		[](bool b) { return b;  }));
}


BOOST_DATA_TEST_CASE(random_test,
	// try arrays of size 0 to 100, and due to randomness, repeat
	// each one 5 times
	boost::unit_test::data::xrange(0, 100) *
	boost::unit_test::data::xrange(0, 5),
	N, test_idx)
{
	vec.resize(N, false);
	MaybeRandomIndex idx(N, true);
	for (size_t i = 0; i < N; ++i, idx.advance())
	{
		size_t j = idx.get();
		BOOST_REQUIRE(j < vec.size());
		BOOST_TEST(!vec[j]);
		vec[j] = true;
	}
	BOOST_TEST(idx.is_end());
	BOOST_TEST(std::all_of(vec.begin(), vec.end(),
		[](bool b) { return b;  }));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


