/**
\file

\author Samuel Barrett

\brief This suite tests the `StoppingIterator`
*/



#include <Internal/StoppingIterator.h>
#include <Internal/FixedStoppingStrategy.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using atp::search::StoppingIterator;
using atp::search::FixedStoppingStrategy;
using atp::search::StoppingStrategyPtr;
using atp::search::IHeuristic;
using atp::search::HeuristicPtr;
using atp::logic::ProofStatePtr;
using atp::logic::PfStateSuccIterPtr;
using atp::logic::StmtFormat;


class ConstantHeuristic :
	public IHeuristic
{
public:
	float predict(const ProofStatePtr&) override
	{
		++m_num_calls;
		return 0.0f;
	}

public:
	size_t m_num_calls = 0;
};


struct StoppingIteratorTestsFixture :
	public LogicSetupFixture
{
	const size_t N = 3;

	ProofStatePtr p_state;
	PfStateSuccIterPtr p_state_iter;

	std::shared_ptr<StoppingIterator> p_stop_iter;
	std::unique_ptr<FixedStoppingStrategy> p_strat =
		std::make_unique<FixedStoppingStrategy>(N);
	std::shared_ptr<ConstantHeuristic> p_heuristic =
		std::make_shared<ConstantHeuristic>();

	StoppingIteratorTestsFixture()
	{
		s << "i(i(x)) = x";

		auto p_stmts = p_lang->deserialise_stmts(s,
			StmtFormat::TEXT, *p_ctx);
		
		p_state = p_ker->begin_proof_of(p_stmts->at(0));
		p_state_iter = p_state->succ_begin();

		p_stop_iter = std::make_shared<StoppingIterator>(
			p_state_iter, std::move(p_strat), p_heuristic);
	}
};


BOOST_FIXTURE_TEST_SUITE(StoppingIteratorTests,
	StoppingIteratorTestsFixture,
	* boost::unit_test_framework::depends_on(
	"FixedStoppingStrategyTests"));


BOOST_AUTO_TEST_CASE(size_test)
{
	// indicates that the implementation is following the
	// instructions of the stopping strategy
	BOOST_TEST(p_stop_iter->size() >= N);
	BOOST_TEST(p_heuristic->m_num_calls >= N);
}


BOOST_AUTO_TEST_CASE(test_initially_valid)
{
	// indicates that the implementation is looking into the stopping
	// strategy right away at construction, rather than waiting for
	// get() or advance()
	BOOST_TEST(p_stop_iter->valid());
}


BOOST_AUTO_TEST_CASE(test_advance_adds_extra_item,
	* boost::unit_test_framework::depends_on(
	"StoppingIteratorTests/test_initially_valid"))
{
	const size_t n = p_heuristic->m_num_calls;

	p_stop_iter->advance();

	BOOST_TEST(p_heuristic->m_num_calls == n + 1);
}


BOOST_AUTO_TEST_SUITE_END();


