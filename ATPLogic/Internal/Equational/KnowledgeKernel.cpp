/**

\file

\author Samuel Barrett

*/


#include "KnowledgeKernel.h"
#include <sstream>
#include <algorithm>
#include <boost/functional/hash.hpp>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include "ProofState.h"
#include "StmtSuccIterator.h"


namespace atp
{
namespace logic
{
namespace equational
{


KnowledgeKernel::KnowledgeKernel(const ModelContext& ctx) :
	m_context(ctx),
	m_next_thm_ref_id(1),
	m_active_rules(nullptr)
{ }


KnowledgeKernelPtr KnowledgeKernel::try_construct(
	const Language& lang, const ModelContext& ctx)
{
	auto p_ker = std::make_shared<KnowledgeKernel>(ctx);

	std::stringstream s;

	for (size_t i = 0; i < ctx.num_axioms(); ++i)
	{
		s << ctx.axiom_at(i) << std::endl;
	}

	p_ker->m_axioms = lang.deserialise_stmts(s,
		StmtFormat::TEXT, ctx);

	if (p_ker->m_axioms == nullptr)
		return KnowledgeKernelPtr();

	p_ker->rebuild_matchings();

	return p_ker;
}


size_t KnowledgeKernel::get_integrity_code() const
{
	// it is important that we include the arity in this check
	// as well as the name

	const auto& id_to_arity_map = m_context.id_to_arity_map();

	size_t hash = boost::hash_range(
		id_to_arity_map.begin(), id_to_arity_map.end());
	
	// kind of hacky: convert all theorems to strings, then hash
	// those strings

	// warning: this is testing for identical rules, not equivalent
	// rules (which allow free variable permutations). Are we sure
	// this is what we want?

	std::vector<std::string> rules_str;
	rules_str.reserve(m_active_rules->size());
	std::transform(m_active_rules->begin(),
		m_active_rules->end(),
		std::back_inserter(rules_str),
		boost::bind(&Statement::to_str, _1));

	hash += boost::hash_range(rules_str.begin(),
		rules_str.end());

	return hash;
}


ProofStatePtr KnowledgeKernel::begin_proof_of(
	const IStatement& _stmt, IterSettings flags) const
{
	ATP_LOGIC_PRECOND(iter_settings_supported(flags));

	// make sure it's one of "our" statements
	const Statement* p_stmt = dynamic_cast<const Statement*>(
		&_stmt);

	ATP_LOGIC_PRECOND(p_stmt != nullptr);
	ATP_LOGIC_PRECOND(type_check(m_context, *p_stmt));

	// here is our default value for iter settings
	if (flags == iter_settings::DEFAULT)
		flags = iter_settings::NO_REPEATS;

	// extract values from the flags to pass to the ProofState which
	// is constructed below
	const bool no_repeats = ((flags & iter_settings::NO_REPEATS) != 0);
	const bool randomised = ((flags & iter_settings::RANDOMISED) != 0);

	return std::make_shared<ProofState>(m_context,
		*this, *p_stmt, no_repeats, randomised);
}

StmtSuccIterPtr KnowledgeKernel::begin_succession_of(const IStatement& _stmt) const
{
	const Statement* p_stmt = dynamic_cast<const Statement*>(
		&_stmt);

	ATP_LOGIC_PRECOND(p_stmt != nullptr);
	ATP_LOGIC_PRECOND(type_check(m_context, *p_stmt));

	return std::make_shared<StmtSuccIterator>(*p_stmt, m_context, *this);
}


bool KnowledgeKernel::iter_settings_supported(
	IterSettings flags) const
{
	// only support No Repeats and Randomisation
	return flags == (flags & (iter_settings::NO_REPEATS
		| iter_settings::RANDOMISED | iter_settings::DEFAULT));
}


bool KnowledgeKernel::is_trivial(
	const IStatement& _stmt) const
{
	const Statement* p_stmt = dynamic_cast<const Statement*>(
		&_stmt);

	ATP_LOGIC_PRECOND(p_stmt != nullptr);
	ATP_LOGIC_PRECOND(type_check(m_context, *p_stmt));

	// "implies" here is necessary for unidirectional truths,
	// note that we could also check "equivalent", however this is
	// redundant as if two expressions are equivalent, then either
	// of them implies the other.
	// "reflexivity" is for the last case of trivialities

	return std::any_of(m_active_rules->begin(),
		m_active_rules->end(),
		boost::bind(&Statement::implies, _1, boost::ref(*p_stmt)))
		|| p_stmt->true_by_reflexivity();
}


size_t KnowledgeKernel::add_theorems(StatementArrayPtr p_thms)
{
	ATP_LOGIC_PRECOND(p_thms->size() > 0);
	ATP_LOGIC_PRECOND(p_thms != nullptr);
	ATP_LOGIC_ASSERT(m_theorems.find(m_next_thm_ref_id)
		== m_theorems.end());

#ifdef ATP_LOGIC_DEFENSIVE
	// only type-check the theorems in defensive mode because this is
	// slow
	for (size_t i = 0; i < p_thms->size(); ++i)
	{
		auto p_stmt = dynamic_cast<const Statement*>(&p_thms->at(i));

		ATP_LOGIC_PRECOND(p_stmt != nullptr);
		ATP_LOGIC_PRECOND(type_check(m_context,
			*p_stmt));
	}
#endif

	m_theorems[m_next_thm_ref_id] = p_thms;

	rebuild_matchings();

	return m_next_thm_ref_id++;
}


void KnowledgeKernel::remove_theorems(size_t ref_id)
{
	auto iter = m_theorems.find(ref_id);

	ATP_LOGIC_PRECOND(iter != m_theorems.end());

	m_theorems.erase(iter);

	rebuild_matchings();
}


bool KnowledgeKernel::try_match(size_t match_index,
	const Expression& expr,
	FreeVarMap<Expression>* p_out_subs) const
{
	ATP_LOGIC_PRECOND(match_index < m_matches.size());

	// just delegate this to the Expression::try_match function
	return m_matches.at(match_index).first.try_match(expr,
		p_out_subs);
}


std::vector<std::pair<Expression, FreeVarIdSet>>
KnowledgeKernel::match_results_at(size_t match_index,
	FreeVarMap<Expression> match_subs) const
{
	ATP_LOGIC_PRECOND(match_index < m_matches.size());

	// get all of the free variable IDs which were provided a mapping
	// by the argument `match_subs` before we start modifying it
	FreeVarIdSet free_ids_originally_mapped;
	for (auto sub_iter = match_subs.begin();
		sub_iter != match_subs.end(); ++sub_iter)
		free_ids_originally_mapped.insert(sub_iter.first());

	const auto& input_results = m_matches.at(match_index).second;

	MatchResults results;
	results.reserve(input_results.size());
	
	for (size_t i = 0; i < input_results.size(); ++i)
	{
		// we need to make `match_subs` total with respect to the free
		// variables in each substitution result. Note that we *keep*
		// the `match_subs` between result iterations, because (i) it
		// is more efficient if we don't add loads to the map on each
		// iteration, and (ii) it doesn't matter if `match_subs` gives
		// a substitution for a free variable that is not present in
		// the result (as it will still be a total mapping).
		for (size_t free_id : input_results[i].second)
		{
			if (!match_subs.contains(free_id))
			{
				match_subs.insert(free_id, Expression(m_context,
					free_id, SyntaxNodeType::FREE));
			}
		}

		results.emplace_back(
			// apply substitution
			input_results[i].first.map_free_vars(match_subs),

			// copy this over for now and we will edit it below
			input_results[i].second
		);

		// remove any free variables that were provided substitutions
		// in the original user mapping
		results[i].second.remove_if(boost::bind(&FreeVarIdSet::contains,
			boost::ref(free_ids_originally_mapped), _1));
	}

	return results;
}


bool KnowledgeKernel::type_check(const ModelContext& ctx,
	const Statement& stmt)
{
	// we will use a fold to check validity!

	// eq is valid iff both its sides are valid:
	auto eq_valid = boost::phoenix::arg_names::arg1
		&& boost::phoenix::arg_names::arg2;

	// can't get a free variable wrong:
	auto free_valid = boost::phoenix::val(true);

	auto const_valid = [&ctx](size_t symb_id)
	{
		return ctx.is_defined(symb_id)
			&& ctx.symbol_arity(symb_id) == 0;
	};

	auto func_valid = [&ctx](size_t symb_id,
		std::vector<bool>::iterator child_begin,
		std::vector<bool>::iterator child_end)
	{
		const size_t implied_arity = std::distance(child_begin,
			child_end);

		return ctx.is_defined(symb_id)
			&& ctx.symbol_arity(
				symb_id) == implied_arity
			&& std::all_of(child_begin, child_end,
				// use phoenix for an easy identity function
				boost::phoenix::arg_names::arg1);
	};

	return stmt.fold<bool>(eq_valid, free_valid, const_valid,
		func_valid);
}


void KnowledgeKernel::rebuild_matchings()
{
	// reset info

	m_active_rules = nullptr;
	_m_active_rules.reset();
	m_num_matching_rules = 0;
	m_rule_free_id_bound = 0;

	// compute m_active_rules array

	std::vector<StatementArrayPtr> thms;
	thms.reserve(m_theorems.size());
	for (const auto& thm : m_theorems)
		thms.push_back(thm.second);

	// these shouldn't return nullptr
	_m_active_rules = StatementArray::try_concat(
		*m_axioms,
		*StatementArray::try_concat(
			thms));

	m_active_rules = dynamic_cast<const StatementArray*>(
		_m_active_rules.get());

	ATP_LOGIC_ASSERT(m_active_rules != nullptr);

	for (const auto& rule : *m_active_rules)
	{
		// it is entirely possible that `ids` is empty, because the
		// active rule set includes not only axioms, but theorems
		// as well
		if (rule.num_free_vars() > 0)
		{
			const auto& ids = rule.free_var_ids();
			const size_t max_id = ids.max();
			if (max_id > m_rule_free_id_bound)
				m_rule_free_id_bound = max_id;
		}
	}

	// now it's time to compress m_active_rules into a more optimised
	// storage mechanism

	auto match_add = [this](const Expression& match,
		const Expression& rule)
	{
		auto match_iter = std::find_if(m_matches.begin(),
			m_matches.end(), [&match](const MatchRule& mr)
			{
				return match.equivalent(mr.first);
			});

		FreeVarIdSet rule_free_var_ids(
			rule.free_var_ids().begin(), rule.free_var_ids().end());

		if (match_iter != m_matches.end())
		{
			// add the result to an already existing rule
			match_iter->second.emplace_back(rule, rule_free_var_ids);
		}
		else
		{
			// got a brand new rule
			m_matches.emplace_back(match, MatchResults{
				std::make_pair(rule, rule_free_var_ids) });
		}
	};

	for (auto rule_iter = m_active_rules->begin();
		rule_iter != m_active_rules->end(); ++rule_iter)
	{
		auto lhs = rule_iter->lhs();
		auto rhs = rule_iter->rhs();

		match_add(lhs, rhs);
		match_add(rhs, lhs);
	}

#ifdef ATP_LOGIC_DEFENSIVE
	// every match result array should be nonempty
	ATP_LOGIC_ASSERT(std::all_of(m_matches.begin(), m_matches.end(),
		[](const MatchRule& mr) { return !mr.second.empty(); }));
#endif

	m_num_matching_rules = m_matches.size();
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


