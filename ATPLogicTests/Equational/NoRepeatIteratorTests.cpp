/**
\file

\author Samuel Barrett

\brief This suite tests `atp::logic::equational::NoRepeatIterator`
*/


#include <boost/algorithm/string/join.hpp>
#include <Internal/Equational/NoRepeatIterator.h>
#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include <Internal/Equational/ProofState.h>
#include "../Test.h"
#include "StandardTestFixture.h"


using atp::logic::StmtFormat;
using atp::logic::ProofStatePtr;
using atp::logic::equational::NoRepeatIterator;
using atp::logic::equational::Statement;
using atp::logic::equational::StatementArray;
using atp::logic::equational::SyntaxNodeType;
using atp::logic::equational::ProofState;


namespace std
{
static inline ostream& boost_test_print_type(ostream& os,
	list<string> strs)
{
	os << '[' << boost::algorithm::join(strs, ", ") << ']';

	return os;
}
}  // namespace std


BOOST_AUTO_TEST_SUITE(EquationalTests);
BOOST_FIXTURE_TEST_SUITE(NoRepeatIteratorTests,
	StandardTestFixture,
	* boost::unit_test_framework::depends_on(
		"EquationalTests/SubExprMatchingIteratorTests"));


BOOST_DATA_TEST_CASE(test_no_repeats,
	boost::unit_test::data::make({
		std::list<std::string>{
			"i(e) = e",
			"*(e, i(e)) = e"
		},
		std::list<std::string>{
			"*(*(x, y), z) = w",
			"*(x, *(y, z)) = w",
			"*(x, *(y, z)) = *(w, e)"
		}
		}),
	stmts_strs)
{
	// load all of the statements

	s << boost::algorithm::join(stmts_strs, "\n");

	auto _stmts = lang.deserialise_stmts(s, StmtFormat::TEXT,
		ctx);
	const auto& stmts = dynamic_cast<const StatementArray&>(
		*_stmts);

	// convert it into a stack of proof states which are all linked
	// in order

	std::list<ProofStatePtr> pstates;
	pstates.push_back(std::make_shared<ProofState>(
		ctx, ker, stmts.my_at(0)));
	for (size_t i = 1; i < stmts.size(); ++i)
	{
		pstates.push_back(std::make_shared<ProofState>(
			dynamic_cast<const ProofState&>(*pstates.back()),
			stmts.my_at(i)));
	}

	// this is probably redundant, but it is technically up to the
	// implementation of ProofState to decide whether it uses this -
	// so instead wrap the iterator in a NoRepeatIterator:

	auto pstate_iter = pstates.back()->succ_begin();
	auto iter = NoRepeatIterator::construct(
		dynamic_cast<const ProofState&>(*pstates.back()),
		pstate_iter);

	// now extract everything from this iterator and ensure that
	// no successor is equivalent to anything we have already got on
	// the stack

	while (iter->valid())
	{
		auto _succ_proof_state = iter->get();

		const auto& succ_proof_state = dynamic_cast<
			const ProofState&>(*_succ_proof_state);

		auto succ_stmt = succ_proof_state.forefront();

		// shouldn't be equivalent to anything we had on the stack
		BOOST_TEST(std::none_of(stmts.begin(), stmts.end(),
			boost::bind(&Statement::equivalent, _1,
				boost::ref(succ_stmt))));

		iter->advance();
	}
}


BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();


