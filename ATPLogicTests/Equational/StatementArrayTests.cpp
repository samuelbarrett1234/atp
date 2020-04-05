/*

StatementArrayTests.cpp

This tests the `equational::StatementArray` class, along with its
iterator `equational::StatementArray::iterator`.

*/


#include <sstream>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <ATPLogic.h>
#include "../Test.h"


using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Language;
using atp::logic::equational::KnowledgeKernel;
using atp::logic::StmtFormat;
using atp::logic::equational::compute_slice_size;
using atp::logic::concat;


struct StatementArrayTestsFixture
{
	std::stringstream s;
	KnowledgeKernel ker;
	Language lang;

	StatementArrayTestsFixture()
	{
		s << std::noskipws;

		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(StatementArrayTests,
	StatementArrayTestsFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelDefinitionsTests"));


BOOST_AUTO_TEST_CASE(size_test)
{
	// 5 statements
	s << "x = x \n x = x \n x = x \n x = x \n x = x";

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	BOOST_TEST(p_arr->size() == 5);
	
	auto stmtarr = dynamic_cast<const StatementArray&>(
		*p_arr.get());

	BOOST_TEST(std::distance(stmtarr.begin(), stmtarr.end()));
}


BOOST_AUTO_TEST_CASE(basic_iterator_tests)
{
	// 5 statements
	s << "x = x \n x = x \n x = x \n x = x \n x = x";

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	auto stmtarr = dynamic_cast<const StatementArray&>(
		*p_arr.get());

	BOOST_TEST((stmtarr.begin() < stmtarr.end()));
	BOOST_TEST((stmtarr.begin() <= stmtarr.end()));
	BOOST_TEST(!(stmtarr.begin() > stmtarr.end()));
	BOOST_TEST(!(stmtarr.begin() >= stmtarr.end()));
	BOOST_TEST(!(stmtarr.begin() == stmtarr.end()));
	BOOST_TEST((stmtarr.begin() != stmtarr.end()));

	BOOST_TEST(std::distance(stmtarr.begin(),
		stmtarr.begin()) == 0);
	BOOST_TEST(std::distance(stmtarr.end(),
		stmtarr.end()) == 0);

	BOOST_TEST(!(stmtarr.begin() + 5 < stmtarr.end()));
	BOOST_TEST((stmtarr.begin() + 5 <= stmtarr.end()));
	BOOST_TEST(!(stmtarr.begin() + 5 > stmtarr.end()));
	BOOST_TEST((stmtarr.begin() + 5 >= stmtarr.end()));
	BOOST_TEST((stmtarr.begin() + 5 == stmtarr.end()));
	BOOST_TEST(!(stmtarr.begin() + 5 != stmtarr.end()));

	auto iter = stmtarr.begin();
	iter = stmtarr.end();
	BOOST_TEST((iter == stmtarr.end()));
	iter = stmtarr.begin();
	BOOST_TEST((iter == stmtarr.begin()));

	BOOST_TEST((stmtarr.end() - 5 == stmtarr.begin()));

	BOOST_TEST(((stmtarr.begin() + 2) - 2 == stmtarr.begin()));

	iter = stmtarr.begin();
	std::advance(iter, 5);
	BOOST_TEST((iter == stmtarr.end()));
	std::advance(iter, -5);
	BOOST_TEST((iter == stmtarr.begin()));
}


BOOST_AUTO_TEST_CASE(empty_arr_iterator_tests)
{
	// create empty array
	auto stmtarr = StatementArray(
		std::make_shared<StatementArray::ArrType>());

	BOOST_TEST(stmtarr.size() == 0);

	BOOST_TEST((stmtarr.begin() == stmtarr.end()));
}


BOOST_AUTO_TEST_CASE(test_slice_start_end)
{
	// 3 statements
	s << "x = x \n i(x) = i(x) \n";
	s << "i(i(x)) = i(i(x))";

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	auto stmtarr = dynamic_cast<const StatementArray&>(
		*p_arr.get());

	auto p_slice = stmtarr.slice(1, 2, 1);
	BOOST_TEST(p_slice->size() == 1);
	BOOST_TEST(p_slice->at(0).to_str() == "i(x0) = i(x0)");
}


BOOST_AUTO_TEST_CASE(test_slice_step)
{
	// 3 statements
	s << "x = x \n i(x) = i(x) \n";
	s << "i(i(x)) = i(i(x))";

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	auto stmtarr = dynamic_cast<const StatementArray&>(
		*p_arr.get());

	auto p_slice = stmtarr.slice(0, 3, 2);
	BOOST_TEST(p_slice->size() == 2);
	BOOST_TEST(p_slice->at(1).to_str() == "i(i(x0)) = i(i(x0))");
}


BOOST_DATA_TEST_CASE(test_slice_size_computations,
	// start:
	boost::unit_test::data::make({ 0, 1, 0, 0, 2, 2 }) ^
	// stop:
	boost::unit_test::data::make({ 0, 1, 2, 3, 12, 13 }) ^
	// step:
	boost::unit_test::data::make({ 1, 1, 2, 2, 5, 5 }) ^
	// correct size:
	boost::unit_test::data::make({ 0, 0, 1, 2, 2, 3 }),
	start, end, step, size)
{
	BOOST_TEST(compute_slice_size(start, end, step) == size);
}


BOOST_AUTO_TEST_CASE(concat_test)
{
	// 5 statements
	s << "x = x \n x = x \n x = x \n x = x \n x = x";

	auto p_arr1 = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	// reset stream
	s = std::stringstream();
	s << std::noskipws;

	// 7 statements
	s << "x = x \n x = x \n x = x \n x = x \n x = x \n";
	s << "x = x \n x = x";

	auto p_arr2 = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	auto slice1 = p_arr1->slice(1, 4, 2);
	BOOST_REQUIRE(slice1->size() == 2);

	auto slice2 = p_arr2->slice(0, 7, 3);
	BOOST_REQUIRE(slice2->size() == 3);

	// slice both arrays then concatenate
	auto concat_result = concat(*slice1, *slice2);

	// this is the main thing we're testing
	BOOST_TEST(concat_result->size() == 5);
}


BOOST_AUTO_TEST_SUITE_END();  // StatementArrayTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


