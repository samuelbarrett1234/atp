/**

\file

\author Samuel Barrett

\brief This suite tests the IProofState implementation for
	equational logic.

*/


#include <sstream>
#include <vector>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/ProofState.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/Language.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::equational::ProofState;
using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::StmtFormat;
using atp::logic::StatementArrayPtr;
using atp::logic::ProofStatePtr;
using atp::logic::ProofCompletionState;
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


namespace atp
{
namespace logic
{
static inline std::ostream& boost_test_print_type(std::ostream& os,
	ProofCompletionState st)
{
	switch (st)
	{
	case ProofCompletionState::PROVEN:
		os << "PROVEN";
		break;
	case ProofCompletionState::NO_PROOF:
		os << "NO_PROOF";
		break;
	case ProofCompletionState::UNFINISHED:
		os << "UNFINISHED";
		break;
	}
	return os;
}
}  // namespace logic
}  // namespace atp


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(ProofStateTests,
	StandardTestFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SubExprMatchingIteratorTests"));


BOOST_DATA_TEST_CASE(test_automatically_proven,
	boost::unit_test::data::make({
		"*(*(x, y), z) = *(x, *(y, z))",
		"e = e", "x = x", "i(x) = i(x)",
		"e = *(i(y), y)", "*(x, y) = *(x, y)",
		"*(i(x), i(i(x))) = e", "*(e, e) = e",
		}), stmt)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(p_stmts->size() == 1);

	auto pf_state = ker.begin_proof_of(p_stmts->at(0));

	BOOST_TEST(pf_state->completion_state() ==
		ProofCompletionState::PROVEN);
}


BOOST_DATA_TEST_CASE(test_not_automatically_proven,
	boost::unit_test::data::make({
		"i(e) = e",
		"*(x, y) = *(y, x)"
		}), stmt)
{
	s << stmt;

	auto p_stmts = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(p_stmts->size() == 1);

	auto pf_state = ker.begin_proof_of(p_stmts->at(0));

	BOOST_TEST(pf_state->completion_state() ==
		ProofCompletionState::UNFINISHED);
}


// given several <statement, successor> pairs, test that the
// successor appears in the list of successors returned by
// the iterator
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
		}),
	stmt, succ_stmt)
{
	s << stmt << "\n" << succ_stmt;

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(p_arr != nullptr);
	BOOST_REQUIRE(p_arr->size() == 2);

	auto pf_state = ker.begin_proof_of(p_arr->at(0));
	auto target_succ = dynamic_cast<const Statement&>(
		p_arr->at(1));

	// enumerate the successor proofs
	std::vector<ProofStatePtr> succs;
	auto succ_iter = pf_state->succ_begin();
	while (succ_iter->valid())
	{
		succs.emplace_back(succ_iter->get());
		succ_iter->advance();
	}

	BOOST_TEST(std::any_of(succs.begin(),
		succs.end(),
		[&target_succ](ProofStatePtr p_pf)
		{
			return target_succ.equivalent(dynamic_cast<
				ProofState*>(p_pf.get())->forefront());
		}));
}


// given several <statement, successor> pairs, test that the
// not_successor does not appear in the list of successors
// returned by the iterator
BOOST_DATA_TEST_CASE(test_is_not_a_succ,
	boost::unit_test::data::make({
		// starting statements
		"*(x, y) = *(y, x)"
		}) ^
	boost::unit_test::data::make({
		// one of the corresponding successor statements
		"*(x, y) = *(x, y)"
		}),
	stmt, not_succ_stmt)
{
	s << stmt << "\n" << not_succ_stmt;

	auto p_arr = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	BOOST_REQUIRE(p_arr != nullptr);
	BOOST_REQUIRE(p_arr->size() == 2);

	auto pf_state = ker.begin_proof_of(p_arr->at(0));
	auto not_succ = dynamic_cast<const Statement&>(
		p_arr->at(1));

	// enumerate the successor proofs
	std::vector<ProofStatePtr> succs;
	auto succ_iter = pf_state->succ_begin();
	while (succ_iter->valid())
	{
		succs.emplace_back(succ_iter->get());
		succ_iter->advance();
	}

	BOOST_TEST(std::none_of(succs.begin(),
		succs.end(),
		[&not_succ](ProofStatePtr p_pf)
		{
			return not_succ.equivalent(dynamic_cast<
				ProofState*>(p_pf.get())->forefront());
		}));
}



BOOST_AUTO_TEST_CASE(test_tricky_proof)
{
	// this proof is tricky as it relies on an extra theorem:
	s << "e = *(*(x, y), i(*(x, y)))";
	auto extra_thms = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	ker.add_theorems(extra_thms);

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
	auto proof = lang.deserialise_stmts(s, StmtFormat::TEXT, ctx);

	// check that each step follows from the last by appearing as one
	// of its successors
	for (size_t i = 0; i < proof->size() - 1; ++i)
	{
		const auto& premise = proof->at(i);
		const auto& concl = dynamic_cast<const Statement&>(
			proof->at(i + 1));

		auto pf_state = ker.begin_proof_of(premise);

		// enumerate the successor proofs
		std::vector<ProofStatePtr> succs;
		auto succ_iter = pf_state->succ_begin();
		while (succ_iter->valid())
		{
			succs.emplace_back(succ_iter->get());
			succ_iter->advance();
		}

		BOOST_TEST(std::any_of(succs.begin(),
			succs.end(),
			[&concl](ProofStatePtr p_pf)
			{
				return concl.equivalent(dynamic_cast<
					ProofState*>(p_pf.get())->forefront());
			}));
	}
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


