#pragma once


/**

\file

\author Samuel Barrett

\brief Implementation of the IKnowledgeKernel for equational logic

*/


#include <map>
#include <string>
#include <vector>
#include <list>
#include <boost/bimap.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "Statement.h"
#include "StatementArray.h"
#include "ModelContext.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\brief IKnowledgeKernel implementation for equational logic

\details Basically acts as an optimised expression-matcher. **Note**:
	every time you call `add_theorems` or `remove_theorems`, it
	forces a "matching rule rebuild", which is an O(n^2) operation in
	the total number of theorems and axioms, and it builds a set of
	expression matching rules to make it efficient to iterate over
	successor statements etc.
*/
class ATP_LOGIC_API KnowledgeKernel :
	public IKnowledgeKernel
{
public:
	// use builder function instead of constructing this way
	KnowledgeKernel(const ModelContext& ctx);

	/**
	\brief A builder function for this model context implementation.

	\returns `nullptr` on failure, otherwise returns a valid object.

	\note Note that failure can occur due to a model context having
		ill-formed statements.
	*/
	static KnowledgeKernelPtr try_construct(const Language& lang,
		const ModelContext& ctx);

	size_t get_integrity_code() const override;

	ProofStatePtr begin_proof_of(
		const IStatement& stmt) const override;

	bool is_trivial(
		const IStatement& stmt) const override;

	size_t add_theorems(
		StatementArrayPtr p_thms) override;

	void remove_theorems(size_t ref_id) override;

	// not part of the IKnowledgeKernel interface

	inline const StatementArray& get_active_rules() const
	{
		return *m_active_rules;
	}

	/**
	\brief Returns the largest free variable ID which occurs in any
		of the rules.
	*/
	inline size_t get_rule_free_id_bound() const
	{
		return m_rule_free_id_bound;
	}

	inline size_t num_matching_rules() const
	{
		return m_num_matching_rules;
	}

	/**
	\brief Try to match the given expression to the match rules at
		the given index.

	\details The match does not attempt to apply rules recursively;
		instead it tries to apply the rules at the root of `expr`.

	\param match_index The matching rule to try

	\param expr The expression to try matching

	\param p_out_subs An optional output parameter, for extracting
		the matching if successful.

	\returns True if and only if the match was successful.

	\post p_out_subs, if not null, will be given a mapping which is
		total **with respect to the matching expression's free
		variables** - but not necessarily the match results.

	\pre match_index < num_matching_rules()
	*/
	bool try_match(size_t match_index,
		const Expression& expr,
		std::map<size_t, Expression>* p_out_subs) const;

	/**
	\brief Get the possible effects of a given matching and matching
		substitution. (**Returns the results with the substitution
		applied**).

	\pre match_index < num_matching_rules()

	\returns An array of possible resulting matches, where each
		element is an expression (with `match_subs` applied) followed
		by an array of all free variable IDs in the expression
		**which were not substituted**.
	*/
	std::vector<std::pair<Expression,
		std::vector<size_t>>> match_results_at(
			size_t match_index,
			const std::map<size_t, Expression>& match_subs) const;

private:
	/**
	\brief Checks that all symbol IDs and arities agree with those
		defined in the model context.

	\returns true iff success.
	*/
	static bool type_check(const ModelContext& ctx,
		const Statement& stmt);

	/**
	\brief Compile the axioms and theorems into an optimised storage
		of matching rules.

	\post Clears `m_active_rules` and `m_matches` and rebuilds them
		all from scratch. Also updates `m_num_matching_rules` and
		`m_rule_free_id_bound`.
	*/
	void rebuild_matchings();

private:
	const ModelContext& m_context;
	
	StatementArrayPtr m_axioms;

	// mapping from theorem reference ID to the theorem array
	std::map<size_t, StatementArrayPtr> m_theorems;

	// the next ID to assign when `add_theorems` is called
	size_t m_next_thm_ref_id;

	// a compilation of all axioms and added theorems (invariant:
	// this array is always the union of m_axioms and all added
	// theorems)
	// see `rebuild_active_rules`
	StatementArrayPtr _m_active_rules;
	const StatementArray* m_active_rules;

	size_t m_num_matching_rules, m_rule_free_id_bound;

	// an optimised storage of m_active_rules
	typedef std::vector<std::pair<Expression, std::vector<size_t>>>
		MatchResults;
	typedef std::pair<Expression, MatchResults> MatchRule;
	std::vector<MatchRule> m_matches;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


