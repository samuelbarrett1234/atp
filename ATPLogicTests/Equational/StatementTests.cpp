/*

StatementTests.cpp

This file tests the equational::Statement class.

*/


#include <sstream>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Statement.h>
#include "../Test.h"


using atp::logic::equational::Language;
using atp::logic::equational::KnowledgeKernel;
using atp::logic::StmtFormat;
using atp::logic::StmtForm;
using atp::logic::equational::StatementArray;
using atp::logic::equational::Statement;


struct StatementTestsFixture
{
	std::stringstream s;
	KnowledgeKernel ker;
	Language lang;

	StatementTestsFixture()
	{
		s << std::noskipws;

		// load group theory symbols:
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);

		// load group theory rules:
		s << "*(x, *(y, z)) = *(*(x, y), z) \n";
		s << "*(i(x), x) = e \n";
		s << "*(x, i(x)) = e \n";
		s << "*(x, e) = x \n";
		s << "*(e, x) = x";

		auto p_rules = lang.deserialise_stmts(s,
			StmtFormat::TEXT, ker);

		ker.define_eq_rules(p_rules);

		// reset stream:
		s = std::stringstream();
		s << std::noskipws;
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(StatementTests,
	StatementTestsFixture,
	*boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelDefinitionsTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ParseTreeToSyntaxTreeTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/ParseDefinitionsTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFoldTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SyntaxTreeFreeVarTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests"));


BOOST_DATA_TEST_CASE(test_statement_form,
	boost::unit_test::data::make({ "x = x",
		"i(x) = i(x)", "x = y", "*(x, y) = *(y, x)",
		"*(e, x) = x" }) ^
	boost::unit_test::data::make({ true, true,
		false, false, true}), stmt, is_trivially_true)
{
	s << stmt;

	auto stmt_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	BOOST_TEST(stmt_arr != nullptr);

	if (is_trivially_true)
	{
		BOOST_TEST((stmt_arr->at(0).form() ==
			StmtForm::CANONICAL_TRUE));
	}
	else
	{
		BOOST_TEST((stmt_arr->at(0).form() ==
			StmtForm::NOT_CANONICAL));
	}

	// equational statements cannot be canonically false
}


// only test this with one free variable due to naming issues
// of free variables
BOOST_DATA_TEST_CASE(test_statement_with_one_var_to_str,
	boost::unit_test::data::make({ "x0 = x0",
		"i(x0) = i(x0)", "e = e",
		"*(e, x0) = x0" }), stmt)
{
	// test that .to_str is the inverse of the parser

	s << stmt;

	auto stmt_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	BOOST_TEST(stmt_arr != nullptr);

	BOOST_TEST(stmt_arr->at(0).to_str() == stmt);
}


BOOST_AUTO_TEST_SUITE_END();  // StatementTests
BOOST_AUTO_TEST_SUITE_END();  // EquationalTests


