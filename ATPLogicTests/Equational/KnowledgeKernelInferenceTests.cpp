/**

\file

\author Samuel Barrett

\details This suite tests the inference capabilities of the
    knowledge kernel; for example, can it accurately generate the
	successors of a particular statement? Can it accurately check the
	correctness of a proof? Etc.
    It does not test the definition storage functionality of the
    knowledge kernel; that is done in KnowledgeKernelDefinitionsTests.cpp
    More specifically, we are only interested in testing the functions:
     - `succs`
     - `follows`
     - `get_form`

*/


#include <sstream>
#include <vector>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/Language.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Semantics.h>
#include "../Test.h"


using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::Language;
using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::StatementArrayPtr;
namespace semantics = atp::logic::equational::semantics;
using atp::logic::StmtFormat;
using atp::logic::StmtForm;
namespace phxargs = boost::phoenix::arg_names;


namespace std
{
static inline ostream& boost_test_print_type(ostream& os,
	list<string> strs)
{
	os << '[' << boost::algorithm::join(strs, ", ") << ']';

	return os;
}
}  // namespace std


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
		"e = *(i(y), y)", "*(x, y) = *(x, y)",
		"*(i(x), i(i(x))) = e", "*(e, e) = e",
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
		"i(e) = e", 
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


// given several <statement, successor> pairs, test that the
// successor appears in the list of successors returned by
// the kernel function
BOOST_DATA_TEST_CASE(test_is_a_succ,
	boost::unit_test::data::make({
		// starting statements
		"x = i(x)",
		"x = *(x, i(x))",
		"*(*(x, y), i(*(x, y))) = *(*(x, y), *(i(y), i(x)))",
		"*(*(x, y), i(*(x, y))) = *(x, i(x))",
		}) ^
	boost::unit_test::data::make({
		// one of the corresponding successor statements
		"x = *(e, i(x))",
		"x = e",
		"*(*(x, y), i(*(x, y))) = *(x, *(y, *(i(y), i(x))))",
		"*(*(x, y), i(*(x, y))) = *(x, *(e, i(x)))",
		}), stmt, succ_stmt)
{
	s << stmt << "\n" << succ_stmt;

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ker);

	BOOST_REQUIRE(p_arr != nullptr);
	BOOST_REQUIRE(p_arr->size() == 2);

	auto succ_arr = ker.succs(p_arr->slice(0, 1, 1));
	auto target_succ = dynamic_cast<const Statement&>(
		p_arr->at(1));

	BOOST_REQUIRE(succ_arr.size() == 1);
	BOOST_REQUIRE(succ_arr.front()->size() > 0);

	auto succ = dynamic_cast<const StatementArray&>(
		*succ_arr.front());

	BOOST_TEST(std::any_of(succ.begin(),
		succ.end(),
		boost::bind(&semantics::equivalent, _1,
			boost::ref(target_succ))));
}


BOOST_TEST_DECORATOR(*boost::unit_test_framework::depends_on(
	"EquationalTests/KnowledgeKernelInferenceTests/test_is_a_succ"))
BOOST_DATA_TEST_CASE(check_follows,
boost::unit_test::data::make({
	std::list<std::string>{
		// proof that i(i(x)) = x
		"i(i(x)) = x",
		"*(i(i(x)), e) = x",
		"*(i(i(x)), *(i(x), x)) = x",
		"*(*(i(i(x)), i(x)), x) = x",
		"*(e, x) = x"
	},
	std::list<std::string>{
		// proof that e = i(e)
		"e = i(e)",
		"e = *(e, i(e))"
	}
}),
proof_strs)
{
	s << boost::algorithm::join(proof_strs, "\n");
	auto proof = lang.deserialise_stmts(s, StmtFormat::TEXT, ker);

	auto results = ker.follows(proof->slice(0, proof->size() - 1, 1),
		proof->slice(1, proof->size(), 1));

	BOOST_TEST(std::all_of(results.begin(), results.end(),
		phxargs::arg1));
}


BOOST_AUTO_TEST_CASE(test_tricky_proof)
{
	// this proof is tricky as it relies on an extra theorem:
	s << "e = *(*(x, y), i(*(x, y)))";
	auto extra_thms = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ker);
	ker.define_eq_rules(extra_thms);
	
	// clear this:
	s = std::stringstream();
	s << std::noskipws;

	std::list<std::string> proof_strs = {
		// proof that i(*(x, y)) = *(i(y), i(x))
		"i(*(x, y)) = *(i(y), i(x))",
		"i(*(x, y)) = *( *(i(y), i(x)) , e )",
		"i(*(x, y)) = *( *(i(y), i(x)) , *(*(x, y), i(*(x, y))) )",
		"i(*(x, y)) = *( i(y), *(i(x), *(*(x, y), i(*(x, y)))) )",
		"i(*(x, y)) = *( i(y), *(i(x), *(x, *(y, i(*(x, y))))) )",
		"i(*(x, y)) = *( i(y), *(*(i(x), x),  *(y, i(*(x, y)))) )",
		"i(*(x, y)) = *( i(y), *(e,  *(y, i(*(x, y)))) )",
		"i(*(x, y)) = *( i(y), *(y, i(*(x, y))) )",
		"i(*(x, y)) = *( *(i(y), y), i(*(x, y)) )",
		"i(*(x, y)) = *( e, i(*(x, y)) )",
		"i(*(x, y)) = i(*(x, y))",
	};

	s << boost::algorithm::join(proof_strs, "\n");
	auto proof = lang.deserialise_stmts(s, StmtFormat::TEXT, ker);

	auto results = ker.follows(proof->slice(0, proof->size() - 1, 1),
		proof->slice(1, proof->size(), 1));

	BOOST_TEST(std::all_of(results.begin(), results.end(),
		phxargs::arg1));
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


