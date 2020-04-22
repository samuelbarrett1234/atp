/**

\file

\author Samuel Barrett

\brief Unit tests for the IterativeDeepeningSolver class.

*/


#include <sstream>
#include <boost/phoenix.hpp>
#include <ATPLogic.h>
#include <Internal/IterativeDeepeningSolver.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using atp::search::IterativeDeepeningSolver;
using atp::logic::LanguagePtr;
using atp::logic::ModelContextPtr;
using atp::logic::KnowledgeKernelPtr;
using atp::logic::LangType;
using atp::logic::StmtFormat;
using atp::logic::create_language;
using atp::logic::ProofCompletionState;
using atp::logic::IterSettings;
namespace iter_settings = atp::logic::iter_settings;
namespace phxarg = boost::phoenix::arg_names;


// all the different combinations of flags we will try testing
static const IterSettings iter_flags[] =
{
	iter_settings::DEFAULT,
	iter_settings::NO_REPEATS,
	iter_settings::RANDOMISED,
	iter_settings::NO_REPEATS | iter_settings::RANDOMISED,
	// note: repeating these a couple of times has the effect of
	// repeating tests that include randomness (which is a good
	// idea)
	iter_settings::RANDOMISED,
	iter_settings::NO_REPEATS | iter_settings::RANDOMISED,
	iter_settings::RANDOMISED,
	iter_settings::NO_REPEATS | iter_settings::RANDOMISED
};


struct IterativeDeepeningSolverTestsFixture :
	public LogicSetupFixture
{
	std::unique_ptr<IterativeDeepeningSolver> p_ids;
};


BOOST_FIXTURE_TEST_SUITE(IterativeDeepeningSolverTests,
	IterativeDeepeningSolverTestsFixture);


BOOST_DATA_TEST_CASE(simple_proof_test,
	boost::unit_test::data::make(iter_flags),
	flags)
{
	// create solver
	p_ids = std::make_unique<IterativeDeepeningSolver>(p_ker,
		/* max_depth */ 10, /* starting_depth */ 3, flags);

	// provide the system with an array of statements to try to prove
	// and which it SHOULD be able to prove in a relatively small
	// number of steps, and then run the solver for many iterations
	// until it finds those proofs.

	s << "x = *(x, e) \n";  // trivial
	s << "*(x, y) = *(*(x, y), e) \n";  // still easy
	s << "e = *(i(*(x, y)), *(x, y)) \n";  // still easy
	s << "i(e) = e \n";  // reasonable
	s << "*(x, e) = *(e, x) \n";  // reasonable
#ifndef _DEBUG
	s << "i(i(x)) = x";  // a bit harder (too hard for debug mode!)
#endif

	auto stmts = p_lang->deserialise_stmts(s,
		StmtFormat::TEXT, *p_ctx);

	BOOST_REQUIRE(stmts != nullptr);

	BOOST_TEST(!p_ids->engaged());

	p_ids->set_targets(stmts);

	BOOST_TEST(p_ids->engaged());

	// none of these proofs should take more than 1000 node
	// expansions (I think)
	p_ids->step(
#ifndef _DEBUG
		20000  // this is waaaaay too slow for debug mode
#else
		100
#endif
	);

	auto proofs = p_ids->get_proofs();
	auto pf_states = p_ids->get_states();

	BOOST_TEST(proofs.size() == stmts->size());
	BOOST_TEST(pf_states.size() == stmts->size());

	BOOST_TEST(std::all_of(proofs.begin(), proofs.end(),
		phxarg::arg1 != nullptr));
	BOOST_TEST(std::all_of(pf_states.begin(), pf_states.end(),
		phxarg::arg1 == ProofCompletionState::PROVEN));

	BOOST_TEST(p_ids->engaged());
}


BOOST_DATA_TEST_CASE(false_statement_tests,
	boost::unit_test::data::make(iter_flags),
	flags)
{
	// create solver
	p_ids = std::make_unique<IterativeDeepeningSolver>(p_ker,
		/* max_depth */ 10, /* starting_depth */ 3, flags);

	// a selection of false statements
	s << "*(x, y) = *(y, x) \n";
	s << "x = e \n";
	s << "i(x) = x \n";
	s << "i(x) = e \n";

	auto stmts = p_lang->deserialise_stmts(s,
		StmtFormat::TEXT, *p_ctx);

	p_ids->set_targets(stmts);

	// no proof should be found in any of these steps
	p_ids->step(
#ifndef _DEBUG
		1000  // this is waaaaay too slow for debug mode
#else
		100
#endif
	);

	auto proofs = p_ids->get_proofs();
	auto states = p_ids->get_states();

	BOOST_TEST(proofs.size() == stmts->size());
	BOOST_TEST(states.size() == stmts->size());

	BOOST_TEST(std::none_of(states.begin(),
		states.end(), phxarg::arg1 ==
		ProofCompletionState::PROVEN));

	BOOST_TEST(std::all_of(proofs.begin(),
		proofs.end(), phxarg::arg1 == nullptr));
}


BOOST_AUTO_TEST_SUITE_END();


