/*
\file

\author Samuel Barrett

\brief This suite tests the IteratorManager class.

*/


#include <Internal/IteratorManager.h>
#include <Internal/FixedStoppingStrategy.h>
#include <Internal/BasicStoppingStrategy.h>
#include "Test.h"
#include "LogicSetupFixture.h"


using atp::search::IteratorManager;
using atp::search::FixedStoppingStrategy;
using atp::search::BasicStoppingStrategy;
using atp::logic::StmtFormat;
using atp::logic::ProofStatePtr;
using atp::search::IHeuristic;


// used to track the number of calls to a genuine heuristic, and also
// to return specific values
class DummyHeuristic :
	public IHeuristic
{
public:
	float predict(const ProofStatePtr& p_state) override
	{
		++m_num_calls;
		return m_val;
	}

	size_t m_num_calls = 0;
	float m_val = 0.0f;
};


struct IteratorManagerTestsFixture :
	public LogicSetupFixture
{
	std::unique_ptr<IteratorManager> p_iter_mgr;
	ProofStatePtr p_state;
	
	IteratorManagerTestsFixture() :
		p_iter_mgr(std::make_unique<IteratorManager>(
			p_ker))
	{
		s << "i(i(x)) = x";

		auto p_stmts = p_lang->deserialise_stmts(s, StmtFormat::TEXT,
			*p_ctx);

		p_state = p_ker->begin_proof_of(p_stmts->at(0));
	}
};


BOOST_FIXTURE_TEST_SUITE(IteratorManagerTests,
	IteratorManagerTestsFixture);


BOOST_AUTO_TEST_CASE(test_no_stopping_strategy)
{
	auto p_heuristic = std::make_shared<DummyHeuristic>();

	p_iter_mgr->set_heuristic(p_heuristic);

	auto iter = p_iter_mgr->begin_iteration_of(p_state);

	while (iter->valid())
		iter->advance();

	// this is just an approximate way of checking that no stopping
	// strategy was used
	BOOST_TEST(p_heuristic->m_num_calls == 0);
}


BOOST_AUTO_TEST_CASE(test_reset_stopping_strategy)
{
	auto p_heuristic = std::make_shared<DummyHeuristic>();

	p_iter_mgr->set_heuristic(p_heuristic);

	// set it and then unset it
	p_iter_mgr->set_fixed_stopping_strategy(6);
	p_iter_mgr->reset_stopping_strategy();
	
	auto iter = p_iter_mgr->begin_iteration_of(p_state);

	while (iter->valid())
		iter->advance();

	// this is just an approximate way of checking that no stopping
	// strategy was used
	BOOST_TEST(p_heuristic->m_num_calls == 0);
}


BOOST_AUTO_TEST_CASE(test_fixed_stopping_strategy)
{
	const size_t N = 7;

	p_iter_mgr->set_fixed_stopping_strategy(N);

	auto p_heuristic = std::make_shared<DummyHeuristic>();

	p_iter_mgr->set_heuristic(p_heuristic);

	auto iter = p_iter_mgr->begin_iteration_of(p_state);

	iter->valid();  // calling this should load in N succs

	BOOST_TEST(p_heuristic->m_num_calls == N);
}


BOOST_AUTO_TEST_CASE(test_basic_stopping_strategy)
{
	const size_t N = 3;
	const float lambda = 10.0f;
	const float alpha = 0.1f;

	p_iter_mgr->set_basic_stopping_strategy(N, lambda, alpha);

	auto p_heuristic = std::make_shared<DummyHeuristic>();
	p_heuristic->m_val = 1.0f;

	p_iter_mgr->set_heuristic(p_heuristic);

	auto iter = p_iter_mgr->begin_iteration_of(p_state);

	iter->valid();  // calling this should load in N succs

	BOOST_TEST(p_heuristic->m_num_calls == N);

	p_heuristic->m_val = 10.0f;
	p_heuristic->m_num_calls = 0;  // reset

	// should've had high enough value to suggest that others won't
	// be better, so should've only evaluated one more node
	// (clear the three nodes we initialised with)
	iter->advance();
	iter->advance();
	iter->advance();
	BOOST_TEST(p_heuristic->m_num_calls == 1);
}


BOOST_AUTO_TEST_SUITE_END();


