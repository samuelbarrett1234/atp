/*

KnowledgeKernelInferenceTests.cpp

This suite tests the inference capabilities of the knowledge kernel;
for example, can it accurately generate the successors of a
particular statement? Can it accurately check the correctness of a
proof? Etc.

It does not test the definition storage functionality of the
knowledge kernel; that is done in KnowledgeKernelDefinitionsTests.cpp

More specifically, we are only interested in testing the functions:
- `succs`
- `follows`
- `get_form`

*/


#include <sstream>
#include <boost/phoenix.hpp>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include "../Test.h"


using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::Language;
using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::StmtFormat;
using atp::logic::StmtForm;
namespace phxargs = boost::phoenix::arg_names;


struct KnowledgeKernelInferenceTestsFixture
{
	Language lang;
	KnowledgeKernel ker;
	std::stringstream s;

	KnowledgeKernelInferenceTestsFixture()
	{
		s << std::noskipws;

		// group theory definitions
		ker.define_symbol("e", 0);
		ker.define_symbol("i", 1);
		ker.define_symbol("*", 2);

		// group theory rules
		s << "*(x, e) = x\n";
		s << "*(e, x) = x\n";
		s << "*(x, i(x)) = e\n";
		s << "*(i(x), x) = e\n";
		s << "*(x, *(y, z)) = *(*(x, y), z)";

		auto arr = lang.deserialise_stmts(s,
			StmtFormat::TEXT, ker);

		ker.define_eq_rules(arr);

		// reset this
		s = std::stringstream();
		s << std::noskipws;
	}
};


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(KnowledgeKernelInferenceTests,
	KnowledgeKernelInferenceTestsFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SemanticsTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/LanguageTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/StatementArrayTests")
	* boost::unit_test_framework::depends_on(
		"EquationalTests/KnowledgeKernelDefinitionsTests"));


BOOST_DATA_TEST_CASE(test_form_canonical_true,
	boost::unit_test::data::make({
		"*(*(x, y), z) = *(x, *(y, z))",
		"e = e", "x = x", "i(x) = i(x)",
		"e = *(i(y), y)", "*(x, y) = *(x, y)"
		}), stmt)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	auto forms = ker.get_form(p_stmts);

	BOOST_TEST(std::all_of(forms.begin(),
		forms.end(), phxargs::arg1 == StmtForm::CANONICAL_TRUE));
}


BOOST_DATA_TEST_CASE(test_form_not_canonical,
	boost::unit_test::data::make({
		"*(i(x), i(i(x))) = e",
		"i(e) = e", "*(e, e) = e",
		"*(x, y) = *(y, x)"
		}), stmt)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	auto forms = ker.get_form(p_stmts);

	BOOST_TEST(std::all_of(forms.begin(),
		forms.end(), phxargs::arg1 == StmtForm::NOT_CANONICAL));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


