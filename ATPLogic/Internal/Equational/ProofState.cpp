/**

\file

\author Samuel Barrett

*/


#include <boost/algorithm/string/join.hpp>
#include "ProofState.h"
#include "SubExprMatchingIterator.h"
#include "NoRepeatIterator.h"


namespace atp
{
namespace logic
{
namespace equational
{


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target, Statement current,
	bool no_repeats, bool randomised) :
	m_ctx(ctx), m_ker(ker),
	m_proof(std::make_shared<StmtList>(std::move(target))),
	m_no_repeats(no_repeats), m_randomised(randomised)
{
	m_proof = std::make_shared<StmtList>(std::move(current),
		std::move(m_proof));
	check_forefront_ids();
}


ProofState::ProofState(const ModelContext& ctx,
	const KnowledgeKernel& ker,
	Statement target,
	bool no_repeats, bool randomised) :
	m_proof(std::make_shared<StmtList>(std::move(target))),
	m_ker(ker), m_ctx(ctx),
	m_no_repeats(no_repeats), m_randomised(randomised)
{
	check_forefront_ids();
}


ProofState::ProofState(const ProofState& parent,
	Statement forefront) :
	m_ctx(parent.m_ctx), m_ker(parent.m_ker),
	m_proof(std::make_shared<StmtList>(std::move(forefront),
		parent.m_proof)),
	m_no_repeats(parent.m_no_repeats),
	m_randomised(parent.m_randomised)
{
	check_forefront_ids();
}


PfStateSuccIterPtr ProofState::succ_begin() const
{
	if (m_next_begin_iter == nullptr)
	{
		// compute it because we don't have a cache anyways
		return compute_begin();
	}
	else
	{
		// we want to lose ownership of m_next_begin_iter because
		// the user will generally modify the object returned
		// from this function, and that is entirely designed
		// behaviour - hence we cannot hold onto the cache here.

		auto ptr = std::move(m_next_begin_iter);

		// just to make absolutely sure that std::move will erase
		// this from our object
		ATP_LOGIC_ASSERT(m_next_begin_iter == nullptr);

		return ptr;
	}
}


ProofCompletionState ProofState::completion_state() const
{
	// compute and cache this value if we haven't computed it already
	if (!m_comp_state.has_value())
	{
		if (m_ker.is_trivial(forefront()))
			m_comp_state = ProofCompletionState::PROVEN;
		else
		{
			if (m_next_begin_iter == nullptr)
			{
				m_next_begin_iter = compute_begin();
			}

			if (m_next_begin_iter->valid())
				m_comp_state = ProofCompletionState::UNFINISHED;
			else
				// if begin iterator is invalid then we have no
				// successors
				m_comp_state = ProofCompletionState::NO_PROOF;
		}
	}

	return *m_comp_state;
}


std::string ProofState::to_str() const
{
	std::list<std::string> as_strs;

	auto p_list = m_proof.get();
	do
	{
		as_strs.push_front(p_list->head.to_str());
		p_list = p_list->tail.get();
	} while (p_list != nullptr);

	return boost::algorithm::join(as_strs, "\n");
}


void ProofState::check_forefront_ids()
{
	const auto ids = forefront().free_var_ids();
	if (std::any_of(ids.begin(), ids.end(),
		boost::bind(std::less_equal<size_t>(), _1,
			m_ker.get_rule_free_id_bound())))
	{
		// modify forefront (which is exactly just the head element).
		m_proof->head = forefront().increment_free_var_ids(
			m_ker.get_rule_free_id_bound() + 1);
	}
}


PfStateSuccIterPtr ProofState::compute_begin() const
{
	PfStateSuccIterPtr iter = SubExprMatchingIterator::construct(m_ctx, m_ker,
		*this, forefront(), m_randomised);

	// user has indicated they want a no-repeat iterator
	if (m_no_repeats)
	{
		iter = NoRepeatIterator::construct(*this, std::move(iter));
	}

	return iter;
}


std::vector<size_t> ProofState::get_usage(
	const StatementArrayPtr& _p_stmts) const
{
	auto p_stmts = dynamic_cast<const StatementArray*>(
		_p_stmts.get());
	ATP_LOGIC_PRECOND(p_stmts != nullptr);

	std::vector<size_t> results;
	results.resize(p_stmts->size(), 0);

	// go back through the proof
	auto p_list = m_proof.get();
	while (p_list->tail != nullptr)
	{
		// count whether each statement could take us from `pre` to
		// `post`
		const auto& pre = p_list->tail->head;
		const auto& post = p_list->head;
		const size_t max_free_id = std::max(pre.free_var_ids().max(),
			post.free_var_ids().max());

		// count each statement for this pre/post match
		for (size_t i = 0; i < p_stmts->size(); ++i)
		{
			// STRATEGY
			// could we go via statement i to deduce pre from post?
			// we determine this by:
			// does there exist a substitution for one side of stmt i
			// s.t. it matches one subexpr of `pre`, and then after the
			// substitution, produces something identical to `post`?

			const Expression stmt_sides[2] =
			{
				// don't forget to increment the free IDs! This makes
				// it easier for us later
				p_stmts->my_at(i).lhs()
				.increment_free_var_ids(max_free_id),
				p_stmts->my_at(i).rhs()
				.increment_free_var_ids(max_free_id)
			};

			// for every location to look for a substitution...
			for (auto pre_iter = pre.begin(); pre_iter != pre.end();
				++pre_iter)
			{
				// for each side of the statement `i`...
				for (size_t j = 0; j < 2; ++j)
				{
					// try to obtain a match at this position
					FreeVarMap<Expression> sub;
					if (stmt_sides[j].try_match(*pre_iter, &sub))
					{
						// if got a match, carry it through to a
						// substitution

						// extend `sub` to make it total:
						for (auto id_iter =
							p_stmts->my_at(i).free_var_ids().begin();
							id_iter !=
							p_stmts->my_at(i).free_var_ids().end();
							++id_iter)
						{
							// don't forget that, also, we've
							// incremented this:
							const size_t id = *id_iter + max_free_id;
							if (!sub.contains(id))
							{
								sub.insert(id, Expression(m_ctx,
									id, SyntaxNodeType::FREE));
							}
						}

						// perform the substitution on the other side
						// of the match
						auto side_to_sub = stmt_sides
							[1 - j].map_free_vars(sub);

						// finally, insert our result and we'll see
						// what it looks like
						auto result = pre.replace(pre_iter,
							side_to_sub);

						// if the substitution result was what we
						// wanted...
						// (we need to use `implies` here to assign
						// values to any additional free variables
						// which have been introduced)
						if (result.implies(post) ||
							result.implies(post.transpose()))
						{
							++results[i];
						}
					}
				}
			}
		}

		p_list = p_list->tail.get();
	}

	return results;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


