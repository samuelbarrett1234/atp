/**

\file

\author Samuel Barrett

\brief Unit tests for the IterativeDeepeningSolver class.

*/


#include <sstream>
#include <ATPLogic.h>
#include <Internal/IterativeDeepeningSolver.h>
#include "Test.h"
#include "DefinitionStrs.h"



using atp::search::IterativeDeepeningSolver;
using atp::logic::LanguagePtr;
using atp::logic::ModelContextPtr;
using atp::logic::KnowledgeKernelPtr;
using atp::logic::LangType;
using atp::logic::StmtFormat;
using atp::logic::create_language;


struct IterativeDeepeningSolverTestsFixture
{
	IterativeDeepeningSolverTestsFixture()
	{
		// use group theory context
		std::stringstream defn_in(group_theory_definition_str);

		p_lang = create_language(LangType::EQUATIONAL_LOGIC);
		p_ctx = p_lang->try_create_context(defn_in);
		p_ker = p_lang->try_create_kernel(*p_ctx);

		// create solver
		p_ids = std::make_unique<IterativeDeepeningSolver>(p_ker,
			/* max_depth */ 10);
	}

	std::stringstream s;
	LanguagePtr p_lang;
	ModelContextPtr p_ctx;
	KnowledgeKernelPtr p_ker;
	std::unique_ptr<IterativeDeepeningSolver> p_ids;
};


BOOST_FIXTURE_TEST_SUITE(IterativeDeepeningSolverTests,
	IterativeDeepeningSolverTestsFixture);


BOOST_AUTO_TEST_CASE(simple_proof_test,
	// (timeout given in seconds)
	* boost::unit_test::timeout(10)
	)
{
	// provide the system with an array of statements to try to prove
	// and which it SHOULD be able to prove in a relatively small
	// number of steps, and then run the solver for many iterations
	// until it finds those proofs.

	s << "x = *(x, e) \n";  // trivial
	s << "*(x, y) = *(*(x, y), e) \n";  // still easy
	s << "e = *(i(*(x, y)), *(x, y)) \n";  // still easy
	s << "i(e) = e \n";  // reasonable
	s << "*(x, e) = *(e, x)";  // reasonable

	auto stmts = p_lang->deserialise_stmts(s,
		StmtFormat::TEXT, *p_ctx);

	BOOST_REQUIRE(stmts != nullptr);

	BOOST_TEST(!p_ids->engaged());

	p_ids->set_targets(stmts);

	BOOST_TEST(p_ids->engaged());

	while (p_ids->any_proof_not_done())
	{
		p_ids->step(5);
	}

	auto proofs = p_ids->get_proofs();

	BOOST_REQUIRE(proofs.size() == stmts->size());

	for (auto pf : proofs)
		BOOST_TEST(pf.get() != nullptr);

	BOOST_TEST(p_ids->engaged());
}


BOOST_AUTO_TEST_SUITE_END();


