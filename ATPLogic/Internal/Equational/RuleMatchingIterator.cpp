/**

\file

\author Samuel Barrett

*/


#include <boost/phoenix.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include "RuleMatchingIterator.h"
#include "MatchResultsIterator.h"
#include "Expression.h"
#include "Statement.h"
#include "ModelContext.h"
#include "KnowledgeKernel.h"
#include "../FreeVarMap.h"


namespace phxargs = boost::phoenix::arg_names;


namespace atp
{
namespace logic
{
namespace equational
{


std::shared_ptr<RuleMatchingIterator> RuleMatchingIterator::construct(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent,
	const Statement& forefront_stmt,
	const Statement::iterator& sub_expr,
	const std::vector<std::pair<size_t,
	 SyntaxNodeType>>& free_const_enum,
	bool randomised)
{
	return std::make_shared<RuleMatchingIterator>(ctx, ker,
		parent, forefront_stmt, sub_expr, free_const_enum,
		randomised);
}


RuleMatchingIterator::RuleMatchingIterator(
	const ModelContext& ctx, const KnowledgeKernel& ker,
	const ProofState& parent,
	const Statement& forefront_stmt,
	const Statement::iterator& sub_expr,
	const std::vector<std::pair<size_t,
	 SyntaxNodeType>>& free_const_enum,
	bool randomised) :
	m_ctx(ctx), m_ker(ker), m_parent(parent),
	m_forefront_stmt(forefront_stmt), m_sub_expr_iter(sub_expr),
	m_free_const_enum(free_const_enum), m_randomised(randomised),
	m_match_index((randomised && ker.num_matching_rules() > 0) ?
		// avoid %0 here:
		(ker.generate_rand() % ker.num_matching_rules()) : 0)
{
	m_matchings.resize(m_ker.num_matching_rules());
	m_is_no_matching.resize(m_ker.num_matching_rules(), false);

	if (m_ker.num_matching_rules() > 0)
	{
		// construct first matching
		rebuild_current();

		// this will bring us to a position where the current
		// matching is good
		if (m_is_no_matching[m_match_index]
			|| !m_matchings[m_match_index]->valid())
			advance();
	}
}


bool RuleMatchingIterator::valid() const
{
	const bool valid = !(m_match_index ==
		m_ker.num_matching_rules());

#ifdef ATP_LOGIC_DEFENSIVE
	// check that we are valid iff there exists i such that the ith
	// matching has not been marked as having no matching and it is
	// either null (not examined yet) or created and valid
	ATP_LOGIC_ASSERT(valid == std::any_of(boost::make_zip_iterator(
		boost::make_tuple(m_matchings.begin(),
			m_is_no_matching.begin())),
		boost::make_zip_iterator(boost::make_tuple(
			m_matchings.end(), m_is_no_matching.end())),
		[](const auto& tup)
		{ return !tup.get<1>() && (tup.get<0>() == nullptr ||
			tup.get<0>()->valid()); }));
#endif

	return valid;
}


ProofStatePtr RuleMatchingIterator::get() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(!m_is_no_matching[m_match_index]);
	ATP_LOGIC_ASSERT(m_matchings[m_match_index] != nullptr);
	ATP_LOGIC_ASSERT(m_matchings[m_match_index]->valid());

	return m_matchings[m_match_index]->get();
}


void RuleMatchingIterator::advance()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_match_index < m_matchings.size());

	std::vector<bool> is_good;
	is_good.resize(m_matchings.size(), false);
	for (size_t i = 0; i < m_matchings.size(); ++i)
	{
		is_good[i] = (!m_is_no_matching[i] &&
			(m_matchings[i] == nullptr || m_matchings[i]->valid()));
	}

	do
	{
		// if there exists a good entry
		if (std::any_of(is_good.begin(), is_good.end(),
			phxargs::arg1))
		{
			ATP_LOGIC_ASSERT(m_match_index < m_matchings.size());

			do
			{
				// generate a new index FIRST
				if (m_randomised)
				{
					// generate a new random index
					m_match_index = m_ker.generate_rand() %
						m_ker.num_matching_rules();
				}
				// in non-randomised mode we don't advance until the
				// current one becomes invalid
				else if (m_matchings[m_match_index] == nullptr
					|| !m_matchings[m_match_index]->valid())
				{
					// this assertion would not hold above for the
					// randomised mode
					ATP_LOGIC_ASSERT(!is_good[m_match_index]);

					// in non-randomised mode we can save memory and
					// get rid of the last node:
					m_matchings[m_match_index].reset();
					m_is_no_matching[m_match_index] = true;
					// note that, above, we have to alter
					// m_is_no_matching to keep the invariant

					// now move to next rule
					++m_match_index;
				}
			} while (!is_good[m_match_index]);

			// handle the advance of the currently selected rule
			if (m_matchings[m_match_index] != nullptr &&
				m_matchings[m_match_index]->valid())
			{
				m_matchings[m_match_index]->advance();

				if (!m_matchings[m_match_index]->valid())
					is_good[m_match_index] = false;
			}
			else if (m_matchings[m_match_index] == nullptr &&
				!m_is_no_matching[m_match_index])
			{
				rebuild_current();

				ATP_LOGIC_ASSERT(m_is_no_matching[m_match_index] ||
					m_matchings[m_match_index] != nullptr);

				if (m_is_no_matching[m_match_index] ||
					!m_matchings[m_match_index]->valid())
					is_good[m_match_index] = false;
			}
		}
		else
		{
			// indicate exit:
			m_match_index = m_ker.num_matching_rules();
			break;
		}
	} while (!is_good[m_match_index]);
}


size_t RuleMatchingIterator::size() const
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_ASSERT(m_matchings[m_match_index] != nullptr);
	ATP_LOGIC_ASSERT(m_matchings[m_match_index]->valid());

	// count up ALL the memory we're using
	size_t mem = 1;
	for (const auto& ptr : m_matchings)
		if (ptr != nullptr && ptr->valid())
			mem += ptr->size();

	return mem;
}


void RuleMatchingIterator::rebuild_current()
{
	ATP_LOGIC_PRECOND(valid());
	ATP_LOGIC_PRECOND(m_matchings[m_match_index] == nullptr);
	ATP_LOGIC_PRECOND(!m_is_no_matching[m_match_index]);

	FreeVarMap<Expression> match_subs;

	// try to match
	if (!m_ker.try_match(m_match_index,
		*m_sub_expr_iter,
		&match_subs))
	{
		m_is_no_matching[m_match_index] = true;
	}
	else
	{
		m_matchings[m_match_index] = MatchResultsIterator::construct(
			m_ctx, m_ker, m_parent, m_forefront_stmt,
			m_ker.match_results_at(m_match_index,
				std::move(match_subs)),
			m_sub_expr_iter, m_free_const_enum, m_randomised);

		// this could only be invalid if there were no match results
		// from the kernel, but of course, there should be
		ATP_LOGIC_ASSERT(m_matchings[m_match_index]->valid());
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


