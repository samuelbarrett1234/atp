/*

KnowledgeKernelDefinitionsTests.cpp

This file tests some of the functionality of the Language and
KnowledgeKernel classes in the equational namespace. It tests
the functionality for loading definitions etc. It does NOT test
the functionality for the correctness of the implementation of
equational logic, as that relies on syntax trees, which in turn
rely on the functionality tested in this suite.

Basically we are testing the functions in equational::KnowledgeKernel
which are not present in the interface, IKnowledgeKernel, and which
doesn't involve the Statement object.

*/


#include <Internal/Equational/KnowledgeKernel.h>
#include "../Test.h"


using atp::logic::equational::KnowledgeKernel;


struct KnowledgeKernelDefinitionsFixture
{
	KnowledgeKernel ker;
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(KnowledgeKernelDefinitionsTests,
	KnowledgeKernelDefinitionsFixture);


BOOST_AUTO_TEST_CASE(test_define_symbol_and_recall_arity)
{
	ker.define_symbol("f", 2);
	BOOST_REQUIRE(ker.is_defined("f"));
	BOOST_TEST(ker.symbol_arity_from_name("f") == 2);
}


BOOST_AUTO_TEST_CASE(test_define_symbol_and_get_id)
{
	ker.define_symbol("g", 3);
	BOOST_REQUIRE(ker.is_defined("g"));
	const auto id = ker.symbol_id("g");
	BOOST_TEST(ker.id_is_defined(id));
	BOOST_TEST(ker.symbol_arity_from_id(id) == 3);
}


BOOST_AUTO_TEST_SUITE_END();  // KnowledgeKernelDefinitionsTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


