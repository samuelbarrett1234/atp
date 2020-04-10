/**

\file

\author Samuel Barrett

\brief Unit tests for the IterativeDeepeningSolver class.

*/


#include <sstream>
#include "Test.h"
#include <Internal/IterativeDeepeningSolver.h>
#include <ATPLogic.h>


using atp::search::IterativeDeepeningSolver;
using atp::logic::LanguagePtr;
using atp::logic::KnowledgeKernelPtr;
using atp::logic::LangType;
using atp::logic::StmtFormat;
using atp::logic::create_language;


struct IterativeDeepeningSolverTestsFixture
{
	IterativeDeepeningSolverTestsFixture()
	{
		p_lang = create_language(LangType::EQUATIONAL_LOGIC);
		p_ker = p_lang->create_empty_kernel();

		s << std::noskipws;

		// group theory definitions
		s << "e 0 \n i 1 \n * 2";
		p_lang->load_kernel_definitions(*p_ker, s);

		// reset this
		s = std::stringstream();
		s << std::noskipws;

		// group theory rules
		s << "x = *(x, e)\n";
		s << "x = *(e, x)\n";
		s << "e = *(x, i(x))\n";
		s << "e = *(i(x), x)\n";
		s << "*(*(x, y), z) = *(x, *(y, z))";
		p_lang->load_kernel_axioms(*p_ker, s);

		// reset this
		s = std::stringstream();
		s << std::noskipws;

		// create solver
		p_ids = std::make_unique<IterativeDeepeningSolver>(p_ker,
			/* max_depth */ 100);
	}

	std::stringstream s;
	LanguagePtr p_lang;
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
	s << "e = *(i(*(x, y)), *(x, y)) \n";
	s << "i(e) = e \n";  // still reasonable
	s << "*(x, e) = *(e, x)";  // still reasonable

	auto stmts = p_lang->deserialise_stmts(s,
		StmtFormat::TEXT, *p_ker);

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
		BOOST_TEST(pf.has_value());

	// just out of interest:
	auto agg_time = p_ids->get_agg_time();
	auto max_mem = p_ids->get_max_mem();
	auto num_expansions = p_ids->get_num_expansions();

	BOOST_TEST(p_ids->engaged());
}


BOOST_AUTO_TEST_SUITE_END();


