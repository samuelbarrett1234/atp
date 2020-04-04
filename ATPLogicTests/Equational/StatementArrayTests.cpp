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
#include "../Test.h"


using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Language;
using atp::logic::equational::KnowledgeKernel;
using atp::logic::StmtFormat;


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
	BOOST_TEST(iter == stmtarr.begin());

	BOOST_TEST((stmtarr.end() - 5 == stmtarr.begin()));

	iter = stmtarr.begin();
	std::advance(iter, 5);
	BOOST_TEST(iter == stmtarr.end());
	std::advance(iter, -5);
	BOOST_TEST(iter == stmtarr.begin());
}


BOOST_AUTO_TEST_SUITE_END();  // StatementArrayTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


